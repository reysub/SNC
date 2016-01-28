#include "SlavePartitioner.h"

SlavePartitioner::SlavePartitioner(std::map<int, std::string> *mapIdNodes, std::map<int, std::string> *mapIdPreds, std::vector<int> *preds, std::vector<int> *objs, std::map<int, std::vector<int> > *mapNodePos, std::map<int, int> *mapCards)
{
	mappingId2Node = mapIdNodes;
	mappingId2Predicate = mapIdPreds;
	predicates = preds;
	objects = objs;
	mappingNode2Positions = mapNodePos;
	mappingNode2Cardinality = mapCards;
}

void SlavePartitioner::executePartitioning(int processID, int numberOfProcesses, std::string partitioningConfigurationFileURI)
{
	int operationType;
	PartitioningConfigurationFileManager *pcfm = PartitioningConfigurationFileManager::getInstance();
	pcfm->loadConfigurationFile(partitioningConfigurationFileURI);


	MPI_Bcast(&operationType, 1 , MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	if (operationType == TERMINATE_PARTITIONING)
	{
		return;
	}

	else
	{
		createPartitioning(processID, numberOfProcesses);
	}
}



void SlavePartitioner::removeNonBlackListNodes()
{
	std::map<int, std::string> mappingId2Node_NEW;
	std::vector<int> predicates_NEW, objects_NEW;
	std::map<int, std::vector<int> > mappingNode2Positions_NEW;

	for (std::map<int, std::vector<int>>::iterator it = mappingNode2Positions->begin(); it != mappingNode2Positions->end(); ++it)
	{
		if (blackListNodes.find(it->first) != blackListNodes.end())
		{
			for(unsigned int i = 0; i < it->second.size(); ++i)
			{
				if (blackListNodes.find(objects->at(it->second.at(i))) != blackListNodes.end())
				{
					predicates_NEW.push_back(predicates->at(it->second.at(i)));
					objects_NEW.push_back(objects->at(it->second.at(i)));
					if (mappingNode2Positions_NEW.find(it->first) == mappingNode2Positions_NEW.end())
					{
						mappingNode2Positions_NEW.operator [](it->first) = *(new std::vector<int>);
					}
					mappingNode2Positions_NEW[it->first].push_back(predicates_NEW.size() - 1);
				}
			}
		}
	}

	for (int node : blackListNodes)
	{
		if (mappingId2Node->find(node) != mappingId2Node->end())
		{
			mappingId2Node_NEW[node] = mappingId2Node->at(node);
		}
	}

	mappingId2Node->clear();
	mappingNode2Positions->clear();
	predicates->clear();
	objects->clear();
	blackListNodes.clear();
	mappingNode2Cardinality->clear();

	//	mappingId2Node = &mappingId2Node_NEW;
	//	mappingNode2Positions = &mappingNode2Positions_NEW;
	//	predicates = &predicates_NEW;
	//	objects = &objects_NEW;

	predicates->insert(predicates->begin(), predicates_NEW.begin(), predicates_NEW.end());
	objects->insert(objects->begin(), objects_NEW.begin(), objects_NEW.end());
	mappingNode2Positions->insert(mappingNode2Positions_NEW.begin(), mappingNode2Positions_NEW.end());
	mappingId2Node->insert(mappingId2Node_NEW.begin(), mappingId2Node_NEW.end());

	int result = (mappingNode2Positions->empty()) ? 0 : 1;
	MPI_Reduce(&result, NULL, 1, MPI_INT, MPI_MAX, GlobalConstants::MASTER, MPI_COMM_WORLD);
}


void SlavePartitioner::createPartitioning(int processID, int numberOfProcesses)
{
	int operationType;
	std::set<int> visitedNodes;

	int maximumThreshold = PartitioningConfigurationFileManager::getInstance()->getMaximumThreshold();
	int minimumThreshold = PartitioningConfigurationFileManager::getInstance()->getMinimumThreshold();
	initializeBlackListSet(processID, numberOfProcesses, maximumThreshold);

	while (true)
	{
		MPI_Bcast(&operationType, 1 , MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

		if (operationType == GET_SUGGESTION)
		{
			getSuggestion(numberOfProcesses, minimumThreshold, &visitedNodes);
		}

		else if (operationType == ADD_ALL_TO_VISITED)
		{
			addAllToVisited(&visitedNodes);
		}

		else if (operationType == RETRIEVE_TEXTUAL_REPRESENTATION)
		{
			retrieveTextualRepresentationAndTriples();
		}

		else if (operationType == NORMAL_EXECUTION)
		{
			normalExecution(&visitedNodes);
		}

		else if (operationType == TERMINATE_PARTITIONING)
		{
			removeNonBlackListNodes();
			return;
		}
	}
}



void SlavePartitioner::initializeBlackListSet(int processID, int numberOfProcesses, int maximumThreshold)
{
	int nodesSize, blackListVectorSize, *nodes = NULL;	std::vector<int> blackListVector;
	for (std::map<int, int>::iterator it = mappingNode2Cardinality->begin(); it != mappingNode2Cardinality->end(); ++it)
	{
		if (it->second >= maximumThreshold)
		{
			blackListNodes.insert(it->first);
			blackListVector.push_back(it->first);
		}
	}

	for (int i = 1; i < numberOfProcesses; ++i)
	{
		if (i == processID)
		{
			blackListVectorSize = blackListVector.size();
			MPI_Bcast(&blackListVectorSize, 1, MPI_INT, i, MPI_COMM_WORLD);

			if (blackListVectorSize > 0)
			{
				MPI_Bcast(&(blackListVector.at(0)), blackListVectorSize, MPI_INT, i, MPI_COMM_WORLD);
			}
		}

		else
		{
			MPI_Bcast(&nodesSize, 1, MPI_INT, i, MPI_COMM_WORLD);

			if (nodesSize > 0)
			{
				nodes = (int*) realloc(nodes, nodesSize * sizeof(int));
				MPI_Bcast(nodes, nodesSize, MPI_INT, i, MPI_COMM_WORLD);

				for (int j = 0; j < nodesSize; ++j)
				{
					if(mappingNode2Positions->find(nodes[j]) != this->mappingNode2Positions->end())
					{
						blackListNodes.insert(nodes[j]);
					}
				}
			}
		}
	}

	free (nodes);
	blackListVector.clear();
}

void SlavePartitioner::sendVector(std::vector<int> *vector)
{
	int vectorSize = vector->size();
	MPI_Gather(&vectorSize, 1, MPI_INT, NULL, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
	MPI_Gatherv(&vector->at(0), vectorSize, MPI_INT, NULL, NULL, NULL, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
}



void SlavePartitioner::getSuggestion(int numberOfProcesses, int minimumThreshold, std::set<int> *visitedNodes)
{
	int kernelSize; std::set<int> kernelSet;
	MPI_Bcast(&kernelSize, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	if (kernelSize > 0)
	{
		int *kernel = (int*) malloc(kernelSize * sizeof(int));
		MPI_Bcast(kernel, kernelSize, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
		kernelSet.insert(kernel, kernel + kernelSize);
	}

	int position = 0, numberOfNodesToSend = minimumThreshold / numberOfProcesses;

	if (numberOfNodesToSend == 0)
	{
		numberOfNodesToSend = 1;
	}

	int *nodes = (int*)calloc(numberOfNodesToSend, sizeof(int));

	for (std::map<int, std::string>::iterator it = mappingId2Node->begin(); it != mappingId2Node->end(); ++it)
	{
		if (visitedNodes->find(it->first) == visitedNodes->end() && blackListNodes.find(it->first) == blackListNodes.end() && kernelSet.find(it->first) == kernelSet.end() && mappingNode2Positions->find(it->first) != mappingNode2Positions->end())
		{
			nodes[position] = it->first;
			position++;
		}

		if (position == numberOfNodesToSend)
		{
			break;
		}
	}

	MPI_Gather(nodes, numberOfNodesToSend, MPI_INT, NULL, numberOfNodesToSend, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
	free(nodes);
}


void SlavePartitioner::addAllToVisited(std::set<int> *visitedNodes)
{
	int vectorSize;
	MPI_Bcast(&vectorSize, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	int *vector = (int*)malloc(vectorSize * sizeof(int));
	MPI_Bcast(vector, vectorSize, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	for (int i = 0; i < vectorSize; ++i)
	{
		if(mappingNode2Positions->find(vector[i]) != this->mappingNode2Positions->end())
		{
			visitedNodes->insert(vector[i]);
		}
	}

	free(vector);
}



void SlavePartitioner::retrieveTextualRepresentationAndTriples()
{
	int nodesSize;
	MPI_Bcast(&nodesSize, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
	int *nodes = (int*)malloc(nodesSize * sizeof(int));
	MPI_Bcast(nodes, nodesSize, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	std::string translation = "";
	std::vector<int> triples;
	std::set<int> nodesSet;
	nodesSet.insert(nodes, nodes+nodesSize);

	for (int pos = 0; pos < nodesSize; ++pos)
	{
		if (mappingId2Node->find(nodes[pos]) != mappingId2Node->end())
		{
			translation += mappingId2Node->at(nodes[pos]);
		} else {
			translation += MISSING_TRANSLATION_SYMBOL;
		}

		translation += TRANSLATION_SEPARATION_SYMBOL;

		if (mappingNode2Positions->find(nodes[pos]) != mappingNode2Positions->end())
		{
			for (int position : mappingNode2Positions->at(nodes[pos]))
			{
				if (nodesSet.find(objects->at(position)) != nodesSet.end())
				{
					triples.push_back(nodes[pos]);
					triples.push_back(predicates->at(position));
					triples.push_back(objects->at(position));
				}
			}
		}
	}

	int translationSize = translation.size();
	char *translationChar = strdup(translation.c_str());

	MPI_Send(&translationSize, 1, MPI_INT, GlobalConstants::MASTER, 1000, MPI_COMM_WORLD);
	MPI_Send(translationChar, translationSize, MPI_CHAR, GlobalConstants::MASTER, 3000, MPI_COMM_WORLD);




	if (triples.empty()) { triples.push_back(0); triples.push_back(0); triples.push_back(0); }
	sendVector(&triples);




	translation = "";
	MPI_Bcast(&nodesSize, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
	nodes = (int*)realloc(nodes, nodesSize * sizeof(int));
	MPI_Bcast(nodes, nodesSize, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	for (int pos = 0; pos < nodesSize; ++pos)
	{
		if (mappingId2Predicate->find(nodes[pos]) != mappingId2Predicate->end())
		{
			translation += mappingId2Predicate->at(nodes[pos]);
		} else {
			translation += MISSING_TRANSLATION_SYMBOL;
		}

		translation += TRANSLATION_SEPARATION_SYMBOL;
	}

	translationSize = translation.size();
	translationChar = strdup(translation.c_str());

	MPI_Send(&translationSize, 1, MPI_INT, GlobalConstants::MASTER, 1000, MPI_COMM_WORLD);
	MPI_Send(translationChar, translationSize, MPI_CHAR, GlobalConstants::MASTER, 3000, MPI_COMM_WORLD);

	free(nodes);
	free(translationChar);
}



void SlavePartitioner::normalExecution(std::set<int> *visitedNodes)
{
	int kernelSize, borderSize; std::set<int> kernelSet, borderSet;
	std::vector<int> successors, successorsVisited, successorsBlackList;

	MPI_Bcast(&borderSize, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
	int *border = (int*)malloc(borderSize * sizeof(int));
	MPI_Bcast(border, borderSize, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
	borderSet.insert(border, border + borderSize);

	MPI_Bcast(&kernelSize, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
	if (kernelSize > 0)
	{
		int *kernel = (int*)malloc(kernelSize * sizeof(int));
		MPI_Bcast(kernel, kernelSize, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
		kernelSet.insert(kernel, kernel + kernelSize);
		free(kernel);
	}

	for (int pos = 0; pos < borderSize; ++pos)
	{
		if (mappingNode2Positions->find(border[pos]) != mappingNode2Positions->end())
		{
			for (int succIt : mappingNode2Positions->at(border[pos]))
			{
				if (kernelSet.find(objects->at(succIt)) != kernelSet.end() || borderSet.find(objects->at(succIt)) != borderSet.end())// || objects->at(succIt) == 0)
				{
					continue;
				}

				if (visitedNodes->find(objects->at(succIt)) != visitedNodes->end())
				{
					successorsVisited.push_back(objects->at(succIt));
				}

				else if (blackListNodes.find(objects->at(succIt)) != blackListNodes.end())
				{
					successorsBlackList.push_back(objects->at(succIt));
				}

				else
				{
					successors.push_back(objects->at(succIt));
				}
			}
		}
	}

	free(border);

	if (successors.empty()) { successors.push_back(0); }
	if (successorsVisited.empty()) { successorsVisited.push_back(0); }
	if (successorsBlackList.empty()) { successorsBlackList.push_back(0); }

	sendVector(&successors);
	sendVector(&successorsVisited);
	sendVector(&successorsBlackList);


	int nextStepSize;
	MPI_Bcast(&nextStepSize, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
	if (nextStepSize > 0)
	{
		int *nextStep = (int*)malloc(nextStepSize * sizeof(int));
		int *cardinalities = (int*)malloc(nextStepSize * sizeof(int));
		MPI_Bcast(nextStep, nextStepSize, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

		for (int idx = 0; idx < nextStepSize; ++idx)
		{
			if (mappingNode2Cardinality->find(nextStep[idx]) != mappingNode2Cardinality->end())
			{
				cardinalities[idx] = mappingNode2Cardinality->at(nextStep[idx]);
			} else {
				cardinalities[idx] = 0;
			}
		}

		MPI_Reduce(cardinalities, NULL, nextStepSize, MPI_INT, MPI_MAX, GlobalConstants::MASTER, MPI_COMM_WORLD);

		free(nextStep);
		free(cardinalities);
	}
}

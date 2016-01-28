#include "MasterPartitioner.h"

bool MasterPartitioner::checkPartitioningExistence(std::vector<int> *numberOfPartitions, int numberOfIterations)
{
	std::ifstream inputFileStream((PreprocessingFolderStructureManager::getPartitioningFolderPath(true) + "Partitioning.dat").c_str(), std::ios::binary);

	if (!inputFileStream.good())
	{
		return false;
	}

	int numberOfPartitionsPerIteration = 0;
	std::ifstream partitionFileStream;
	std::stringstream iterationStream, partitionStream;

	for (int i = 0; i < numberOfIterations; ++i)
	{
		iterationStream << i;
		inputFileStream.read((char*)&numberOfPartitionsPerIteration, sizeof(int));

		for (int j = 1; j <= numberOfPartitionsPerIteration; ++j)
		{
			partitionStream << j;
			partitionFileStream.open((PreprocessingFolderStructureManager::getPartitioningFolderPath(true) + "Partition_" + iterationStream.str() + "_" + partitionStream.str() + ".dat").c_str(), std::ios::binary);
			if (!partitionFileStream.good())
			{
				partitionFileStream.close();
				inputFileStream.close();
				return false;
			}

			partitionStream.str("");
			partitionFileStream.close();
		}

		iterationStream.str("");
	}

	inputFileStream.close();
	return true;
}

int MasterPartitioner::executePartitioning(int numberOfProcesses, std::string partitioningConfigurationFileURI, int globalIterator)
{
	int operationType, numberOfPartitions;
	PartitioningConfigurationFileManager *pcfm = PartitioningConfigurationFileManager::getInstance();
	if (!pcfm->loadConfigurationFile(partitioningConfigurationFileURI))
	{
		operationType = TERMINATE_PARTITIONING;
		MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
		return 0;
	}

	std::cout << "	Begin Creating Partitioning\n";
	operationType = CREATE_PARTITIONING;
	MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	numberOfPartitions = createPartitioning(numberOfProcesses, globalIterator);
	std::cout << "	Partitioning Creation Complete\n";

	operationType = TERMINATE_PARTITIONING;
	MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	return (removeNonBlackListNodes()) ? numberOfPartitions : -1 * numberOfPartitions;
}

bool MasterPartitioner::removeNonBlackListNodes()
{
	int numberOfBlackListNodes, zero = 0;
	MPI_Reduce(&zero, &numberOfBlackListNodes, 1, MPI_INT, MPI_MAX, GlobalConstants::MASTER, MPI_COMM_WORLD);
	return numberOfBlackListNodes != 0;
}


int MasterPartitioner::createPartitioning(int numberOfProcesses, int globalIterator)
{
	bool forcePartitionStorage;
	int totalAmountOfNodes, numberOfPartitions = 0, borderAndBorderToExpandCardinality, numberOfVisitedNodes = 0, suggestedNode;
	int maximumThreshold = PartitioningConfigurationFileManager::getInstance()->getMaximumThreshold();
	int minimumThreshold = PartitioningConfigurationFileManager::getInstance()->getMinimumThreshold();
	double expansionRatio = PartitioningConfigurationFileManager::getInstance()->getexpansionRatio();
	double expansionPercentage = PartitioningConfigurationFileManager::getInstance()->getexpansionPercentage();
	bool respectMaximumThreshold = PartitioningConfigurationFileManager::getInstance()->isToRespectMaximumThreshold();
	bool respectExpansionPercentage = PartitioningConfigurationFileManager::getInstance()->isToRespectExpansionPercentage();
	bool respectExpansionRatio = PartitioningConfigurationFileManager::getInstance()->isToRespectExpansionRatio();
	int *cachedSuggestions = (int*)malloc((minimumThreshold/numberOfProcesses) * numberOfProcesses * sizeof(int));
	std::vector<int> kernel, borderToExpand, triples;
	std::set<int> kernelSet;
	std::set<int> border, borderVisited, borderBalckList, nextStep, nextStepVisited, nextStepBlackList;
	std::map<int, std::string> mappinNode2Translation, mappinPredicate2Translation;
	std::map<int, int> mappinNode2KernelCardinality, mappingNode2GlobalCardinality;

	cachedSuggestions[0] = -1;
	initializeBlackListSet(numberOfProcesses);

	while (true)
	{
		suggestedNode = 0;
		forcePartitionStorage = false;

		if (border.empty())
		{
			suggestedNode = getSuggestion(numberOfProcesses, minimumThreshold, cachedSuggestions, &kernelSet, &kernel);

			if (suggestedNode == 0)
			{
				forcePartitionStorage = true;
			}

			else
			{
				border.insert(suggestedNode);
			}
		}

		if (!border.empty())
		{
			int operationType = NORMAL_EXECUTION;
			MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

			totalAmountOfNodes = kernel.size() + border.size() + borderVisited.size() + borderBalckList.size();
			selectNodesToExpand(&border, &borderToExpand, &mappinNode2KernelCardinality, &mappingNode2GlobalCardinality, maximumThreshold, totalAmountOfNodes, suggestedNode, expansionPercentage, expansionRatio, respectExpansionPercentage, respectExpansionRatio, respectMaximumThreshold);

			sendVector(&borderToExpand);
			sendVector(&kernel);

			receiveDataAndKernelCardinalities(numberOfProcesses, &mappinNode2KernelCardinality, &nextStep);
			receiveDataAndKernelCardinalities(numberOfProcesses, NULL, &nextStepVisited);
			receiveDataAndKernelCardinalities(numberOfProcesses, NULL, &nextStepBlackList);
			receiveGlobalCardinalities(numberOfProcesses, &nextStep, &mappingNode2GlobalCardinality);

			clearData(&border, &nextStep);
			clearData(&borderVisited, &nextStepVisited);
			clearData(&borderBalckList, &nextStepBlackList);
		}

		totalAmountOfNodes = kernel.size() + border.size() + borderVisited.size() + borderBalckList.size() + borderToExpand.size() + nextStep.size() + nextStepVisited.size() + nextStepBlackList.size();

		if (totalAmountOfNodes <= maximumThreshold && !forcePartitionStorage)
		{
			for (int node : borderToExpand)
			{
				mappinNode2KernelCardinality.erase(node);
				mappingNode2GlobalCardinality.erase(node);
			}

			kernel.insert(kernel.end(), borderToExpand.begin(), borderToExpand.end());
			kernelSet.insert(borderToExpand.begin(), borderToExpand.end());
			border.insert(nextStep.begin(), nextStep.end());
			borderVisited.insert(nextStepVisited.begin(), nextStepVisited.end());
			borderBalckList.insert(nextStepBlackList.begin(), nextStepBlackList.end());
			borderToExpand.clear(); nextStep.clear(); nextStepVisited.clear(); nextStepBlackList.clear();
		}

		//		testPrint(&kernel, &border, &borderVisited, & borderToExpand, & borderBalckList, &nextStep, &nextStepVisited, &nextStepBlackList, &mappinNode2KernelCardinality, &mappingNode2GlobalCardinality);

		if (!kernel.empty() && (totalAmountOfNodes >= minimumThreshold || forcePartitionStorage))
		{
			if (suggestedNode != 0 && borderToExpand.size() == 1 && borderToExpand.at(0) == suggestedNode)
			{
				borderToExpand.clear();
			}

			addAllToVisited(&kernel);

			borderAndBorderToExpandCardinality = border.size() + borderToExpand.size();
			numberOfVisitedNodes += kernel.size();

			borderToExpand.insert(borderToExpand.end(), border.begin(), border.end());
			borderToExpand.insert(borderToExpand.end(), borderVisited.begin(), borderVisited.end());
			borderToExpand.insert(borderToExpand.end(), borderBalckList.begin(), borderBalckList.end());
			borderToExpand.insert(borderToExpand.end(), kernel.begin(), kernel.end());

			retrieveTextualRepresentationAndTriples(numberOfProcesses, &borderToExpand, &mappinNode2Translation, &mappinPredicate2Translation, &triples);

			numberOfPartitions++;
			savePartition(&kernel, &borderVisited, &borderBalckList, &triples, &mappinNode2Translation, &mappinPredicate2Translation, borderAndBorderToExpandCardinality, numberOfPartitions, numberOfVisitedNodes, globalIterator);

			kernel.clear(); borderToExpand.clear(); triples.clear(); kernelSet.clear();
			border.clear(); borderVisited.clear(); borderBalckList.clear();
			nextStep.clear(); nextStepVisited.clear(); nextStepBlackList.clear();
			mappinNode2Translation.clear(); mappinPredicate2Translation.clear();
			mappinNode2KernelCardinality.clear(); mappingNode2GlobalCardinality.clear();
			cachedSuggestions = (int*)realloc(cachedSuggestions, (minimumThreshold/numberOfProcesses) * numberOfProcesses * sizeof(int));
			cachedSuggestions[0] = -1;
		}

		if (forcePartitionStorage)
		{
			break;
		}
	}

	free(cachedSuggestions);
	saveDetails(numberOfPartitions, maximumThreshold, numberOfProcesses, globalIterator);
	return numberOfPartitions;
}


void MasterPartitioner::initializeBlackListSet(int numberOfProcesses)
{
	int nodesSize, *nodes = NULL;
	for (int i = 1; i < numberOfProcesses; ++i)
	{
		MPI_Bcast(&nodesSize, 1, MPI_INT, i, MPI_COMM_WORLD);
		nodes = (int*) realloc(nodes, nodesSize * sizeof(int));
		MPI_Bcast(nodes, nodesSize, MPI_INT, i, MPI_COMM_WORLD);
	}

	free(nodes);
}


void MasterPartitioner::sendVector(std::vector<int> *vector)
{
	int vectorSize = vector->size();
	MPI_Bcast(&vectorSize, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
	if (vectorSize > 0)
	{
		MPI_Bcast(&vector->at(0), vectorSize, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
	}
}

void MasterPartitioner::receiveDataAndKernelCardinalities(int numberOfProcesses, std::map<int, int> *node2KernelCardinality, std::set<int> *nodes)
{
	int receivedArraySize = 1, unityElement = 1;
	int *receivedArraySizePerProcess = (int*) malloc(numberOfProcesses * sizeof(int));
	int *displacements = (int*) malloc(numberOfProcesses * sizeof(int));

	MPI_Gather(&unityElement, 1, MPI_INT, receivedArraySizePerProcess, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	displacements[0] = 0;
	for (int i = 1; i < numberOfProcesses; ++i)
	{
		displacements[i] = displacements[i-1] + receivedArraySizePerProcess [i-1];
		receivedArraySize += receivedArraySizePerProcess[i];
	}

	int *receivedArray = (int*) malloc(receivedArraySize * sizeof(int));
	MPI_Gatherv(&unityElement, 1, MPI_INT, receivedArray, receivedArraySizePerProcess, displacements, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	for (int i = 1; i < receivedArraySize; i++)
	{
		if (receivedArray[i] != 0)
		{
			nodes->insert(receivedArray[i]);

			if (node2KernelCardinality == NULL)
			{
				continue;
			}

			if (node2KernelCardinality != NULL && node2KernelCardinality->find(receivedArray[i]) == node2KernelCardinality->end())
			{
				node2KernelCardinality->operator [](receivedArray[i]) = 0;
			}
			node2KernelCardinality->operator [](receivedArray[i])++;
		}
	}

	free(receivedArraySizePerProcess);
	free(displacements);
	free(receivedArray);
}

void MasterPartitioner::receiveGlobalCardinalities(int numberOfProcesses, std::set<int> *nextStep, std::map<int, int> *node2GlobalCardinality)
{
	std::vector<int> nextStepVector (nextStep->begin(), nextStep->end());
	sendVector(&nextStepVector);

	if (nextStep->empty())
	{
		return;
	}

	int *sendingVector = (int*)calloc(nextStep->size(), sizeof(int));
	int *receivedVector = (int*)malloc(nextStep->size() * sizeof(int));

	MPI_Reduce(sendingVector, receivedVector, nextStep->size(), MPI_INT, MPI_MAX, GlobalConstants::MASTER, MPI_COMM_WORLD);

	for (unsigned int i = 0; i < nextStep->size(); ++i)
	{
		node2GlobalCardinality->operator [](nextStepVector.at(i)) = receivedVector[i];
	}

	nextStepVector.clear();
	free(sendingVector);
	free(receivedVector);
}


void MasterPartitioner::clearData(std::set<int> *set, std::set<int> *setToClear)
{
	if (set->empty() || setToClear->empty())
	{
		return;
	}

	for (int node : *setToClear)
	{
		if (set->find(node) != set->end())
		{
			setToClear->erase(node);
		}
	}
}



int MasterPartitioner::getSuggestion(int numberOfProcesses, int minimumThreshold, int *cachedSuggestions, std::set<int> *kernelSet, std::vector<int> *kernel)
{
	int suggestion = 0;
	int lastPosition = cachedSuggestions[0];
	int numberOfNodesPerProcess = minimumThreshold / numberOfProcesses;
	int totalAmountOfNodes = numberOfNodesPerProcess * numberOfProcesses;

	if (lastPosition != -1)
	{
		for (int i = lastPosition; i < totalAmountOfNodes; ++i)
		{
			if (cachedSuggestions[i] != 0 && kernelSet->find(cachedSuggestions[i]) == kernelSet->end())
			{
				suggestion = cachedSuggestions[i];
				cachedSuggestions[0] = i+1;
				return suggestion;
			}
		}
	}

	int operationType = GET_SUGGESTION;
	MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	int kernelSize = kernel->size();
	MPI_Bcast(&kernelSize, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	if (kernelSize > 0)
	{
		MPI_Bcast(&kernel->at(0), kernelSize, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
	}

	int *zero = (int*)calloc(numberOfNodesPerProcess, sizeof(int));
	MPI_Gather(zero, numberOfNodesPerProcess, MPI_INT, cachedSuggestions, numberOfNodesPerProcess, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	for (int i = numberOfNodesPerProcess; i < totalAmountOfNodes; ++i)
	{
		if (cachedSuggestions[i] != 0)
		{
			suggestion = cachedSuggestions[i];
			cachedSuggestions[0] = i+1;
			free(zero);
			return suggestion;
		}
	}

	free(zero);
	return 0;
}


void MasterPartitioner::selectNodesToExpand(std::set<int> *border, std::vector<int> *nodesToExpand, std::map<int, int> *kernelCardinalities, std::map<int, int> *globalCardinalities, int maxThreshold, int currentAmountOfNodes, int suggestedNode, double expansionPercentage, double expansionRatio, bool respectExpansionPercentage, bool respectExpansionRatio, bool respectMaximumThreshold)
{
	if (suggestedNode != 0 && border->size() == 1 && *border->begin() == suggestedNode)
	{
		nodesToExpand->push_back(suggestedNode);
		border->clear();
		return;
	}

	if (!respectMaximumThreshold && !respectExpansionRatio && respectExpansionPercentage && expansionPercentage == 1)
	{
		nodesToExpand->insert(nodesToExpand->begin(), border->begin(), border->end());
		border->clear();
		return;
	}

	std::vector<int> sortedBorder = sortNodes(border, kernelCardinalities, globalCardinalities);
	nodesToExpand->push_back(sortedBorder.at(0));
	border->erase(sortedBorder.at(0));

	for (unsigned int i = 1; i < sortedBorder.size(); ++i)
	{
		if ((respectMaximumThreshold && (currentAmountOfNodes + globalCardinalities->at(sortedBorder.at(i)) > maxThreshold)) ||
			(respectExpansionRatio && (kernelCardinalities->at(sortedBorder.at(i)) / globalCardinalities->at(sortedBorder.at(i)) > expansionRatio)) ||
			(respectExpansionPercentage && (nodesToExpand->size() / sortedBorder.size() > expansionPercentage)))
		{
			sortedBorder.clear();
			return;
		}

		nodesToExpand->push_back(sortedBorder.at(i));
		border->erase(sortedBorder.at(i));
	}
}

std::vector<int> MasterPartitioner::sortNodes(std::set<int> *border, std::map<int, int> *kernelCardinalities, std::map<int, int> *globalCardinalities)
{
	std::vector<int> sortedBorder;
	sortedBorder.insert(sortedBorder.begin(), border->begin(), border->end());

	int tmp;
	for (unsigned int i = 0; i < sortedBorder.size(); ++i)
	{
		for (unsigned int j = 0; j < sortedBorder.size() - 1; ++j)
		{
			if ((kernelCardinalities->at(sortedBorder.at(j)) / globalCardinalities->at(sortedBorder.at(j))) < (kernelCardinalities->at(sortedBorder.at(j+1)) / globalCardinalities->at(sortedBorder.at(j+1))))
			{
				tmp = sortedBorder.at(j);
				sortedBorder.at(j) = sortedBorder.at(j + 1);
				sortedBorder.at(j + 1) = tmp;
			}
		}
	}

	return sortedBorder;
}


void MasterPartitioner::addAllToVisited(std::vector<int> *kernel)
{
	int operationType = ADD_ALL_TO_VISITED;
	MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
	sendVector(kernel);
}



void MasterPartitioner::retrieveTextualRepresentationAndTriples(int numberOfprocesses, std::vector<int> *nodes, std::map<int, std::string> *mappingNode2Translation, std::map<int, std::string> *mappingPredicate2Translation, std::vector<int> *triples)
{
	int operationType = RETRIEVE_TEXTUAL_REPRESENTATION;
	MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	sendVector(nodes);
	std::vector<int> predicates;

	retrieveNodesTextualRepresentation(numberOfprocesses, nodes, mappingNode2Translation);
	retrieveTriples(numberOfprocesses, nodes, triples, &predicates);
	sendVector(&predicates);
	retrievePredicatesTextualRepresentation(numberOfprocesses, &predicates, mappingPredicate2Translation);
}

void MasterPartitioner::retrieveNodesTextualRepresentation(int numberOfprocesses, std::vector<int> *nodes, std::map<int, std::string> *mappingNode2Translation)
{
	int position;
	int translationLength; char *translationChar = NULL;
	std::string translation, translationElement;

	for (int count = 1; count < numberOfprocesses; ++count)
	{
		position = 0;
		MPI_Recv(&translationLength, 1, MPI_INT, count, 1000, MPI_COMM_WORLD, NULL);

		translationChar = (char*)realloc(translationChar, translationLength * sizeof(char));
		MPI_Recv(translationChar, translationLength, MPI_CHAR, count, 3000, MPI_COMM_WORLD, NULL);

		translation.assign(translationChar);
		translation = translation.substr(0, translationLength);

		while (translation.find(TRANSLATION_SEPARATION_SYMBOL) != translation.npos) {
			translationElement = translation.substr(0, translation.find(TRANSLATION_SEPARATION_SYMBOL));

			if (mappingNode2Translation->find(nodes->at(position)) == mappingNode2Translation->end() && translationElement != MISSING_TRANSLATION_SYMBOL)
			{
				mappingNode2Translation->operator [](nodes->at(position)) = translationElement;
			}

			translation.erase(0, translationElement.length() + TRANSLATION_SEPARATION_SYMBOL.length());
			position++;
		}
	}

	free(translationChar);
}


void MasterPartitioner::retrieveTriples(int numberOfprocesses, std::vector<int> *nodes, std::vector<int> *triples, std::vector<int> *predicates)
{
	int receivedArraySize = 1, unityElement = 1;
	int *receivedArraySizePerProcess = (int*) malloc(numberOfprocesses * sizeof(int));
	int *displacements = (int*) malloc(numberOfprocesses * sizeof(int));
	std::set<int> predicatesSet;

	MPI_Gather(&unityElement, 1, MPI_INT, receivedArraySizePerProcess, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	displacements[0] = 0;
	for (int i = 1; i < numberOfprocesses; ++i)
	{
		displacements[i] = displacements[i-1] + receivedArraySizePerProcess [i-1];
		receivedArraySize += receivedArraySizePerProcess[i];
	}

	int *receivedArray = (int*) malloc(receivedArraySize * sizeof(int));
	MPI_Gatherv(&unityElement, 1, MPI_INT, receivedArray, receivedArraySizePerProcess, displacements, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	for (int i = 1; i < receivedArraySize; i+=3)
	{
		if (receivedArray[i] != 0)
		{
			triples->push_back(receivedArray[i + 0]);
			triples->push_back(receivedArray[i + 1]);
			triples->push_back(receivedArray[i + 2]);
			predicatesSet.insert(receivedArray[i + 1]);
		}
	}

	predicates->insert(predicates->begin(), predicatesSet.begin(), predicatesSet.end());

	free(receivedArraySizePerProcess);
	free(displacements);
	free(receivedArray);
}


void MasterPartitioner::retrievePredicatesTextualRepresentation(int numberOfprocesses, std::vector<int> *predicates, std::map<int, std::string> *mappingPredicates2Translation)
{
	int position;
	int translationLength; char *translationChar = NULL;
	std::string translation, translationElement;

	for (int count = 1; count < numberOfprocesses; ++count)
	{
		position = 0;
		MPI_Recv(&translationLength, 1, MPI_INT, count, 1000, MPI_COMM_WORLD, NULL);

		translationChar = (char*)realloc(translationChar, translationLength * sizeof(char));
		MPI_Recv(translationChar, translationLength, MPI_CHAR, count, 3000, MPI_COMM_WORLD, NULL);

		translation.assign(translationChar);
		translation = translation.substr(0, translationLength);

		while (translation.find(TRANSLATION_SEPARATION_SYMBOL) != translation.npos) {
			translationElement = translation.substr(0, translation.find(TRANSLATION_SEPARATION_SYMBOL));

			if (mappingPredicates2Translation->find(predicates->at(position)) == mappingPredicates2Translation->end() && translationElement != MISSING_TRANSLATION_SYMBOL)
			{
				mappingPredicates2Translation->operator [](predicates->at(position)) = translationElement;
			}

			translation.erase(0, translationElement.length() + TRANSLATION_SEPARATION_SYMBOL.length());
			position++;
		}
	}

	free(translationChar);
}


void MasterPartitioner::savePartition(std::vector<int> *kernel, std::set<int> *borderVisited, std::set<int> *borderBlackList, std::vector<int> *triples, std::map<int, std::string> *nodesTranslation, std::map<int, std::string> *predicatesTranslation, int borderSize, int partitionNumber, int numberOfVisitedNodes, int globalIterator)
{
	int collectionSize, elementLenght;
	std::stringstream partitionNumberStream, globalIteratorStream;
	partitionNumberStream << partitionNumber; globalIteratorStream << globalIterator;
	int alertFrequency = PartitioningConfigurationFileManager::getInstance()->getAlertFrequency();
	std::ofstream outputFileStream((PreprocessingFolderStructureManager::getPartitioningFolderPath(true) + "Partition_" + globalIteratorStream.str() + "_" + partitionNumberStream.str() + ".dat").c_str(), std::ios::binary);

	collectionSize = kernel->size();
	outputFileStream.write((char*)&collectionSize, sizeof(int));
	for (int node : *kernel)
	{
		outputFileStream.write((char*)&node, sizeof(int));
	}

	collectionSize = borderVisited->size();
	outputFileStream.write((char*)&collectionSize, sizeof(int));
	for (int node : *borderVisited)
	{
		outputFileStream.write((char*)&node, sizeof(int));
	}

	collectionSize = borderBlackList->size();
	outputFileStream.write((char*)&collectionSize, sizeof(int));
	for (int node : *borderBlackList)
	{
		outputFileStream.write((char*)&node, sizeof(int));
	}

	collectionSize = nodesTranslation->size();
	outputFileStream.write((char*)&collectionSize, sizeof(int));
	for (std::map<int, std::string>::iterator it = nodesTranslation->begin(); it != nodesTranslation->end(); ++it)
	{
		outputFileStream.write((char*)&it->first, sizeof(int));
		elementLenght = it->second.length();
		outputFileStream.write((char*)&elementLenght, sizeof(int));
		outputFileStream.write(it->second.c_str(), it->second.length());
	}

	collectionSize = predicatesTranslation->size();
	outputFileStream.write((char*)&collectionSize, sizeof(int));
	for (std::map<int, std::string>::iterator it = predicatesTranslation->begin(); it != predicatesTranslation->end(); ++it)
	{
		outputFileStream.write((char*)&it->first, sizeof(int));
		elementLenght = it->second.length();
		outputFileStream.write((char*)&elementLenght, sizeof(int));
		outputFileStream.write(it->second.c_str(), it->second.length());
	}

	collectionSize = triples->size();
	outputFileStream.write((char*)&collectionSize, sizeof(int));
	for (int element : *triples)
	{
		outputFileStream.write((char*)&element, sizeof(int));
	}

	outputFileStream.close();

	outputFileStream.open((PreprocessingFolderStructureManager::getPartitioningFolderPath(false) + "PartitioningRatio").c_str(), std::ios::app);
	outputFileStream << "Partition " << globalIterator << "." << partitionNumber << ": Kernel Size = " << kernel->size() << ", Border Size = " << borderSize << ", BorderVisited Size = " << borderVisited->size() << ", BorderBlackList Size = " << borderBlackList->size() << "\n";
	outputFileStream.close();

	if (partitionNumber % alertFrequency == 0)
	{
		std::cout << "		Created partition " << globalIterator << "." << partitionNumber << " - Number of Visited Nodes = " << numberOfVisitedNodes << "\n";
	}
}

void MasterPartitioner::saveDetails(int numberOfPartitions, int threshold, int numberOfProcesses, int globalIterator)
{
	std::stringstream globalIteratorStream; globalIteratorStream << globalIterator;
	std::ofstream outputFileStream((PreprocessingFolderStructureManager::getPartitioningFolderPath(true) + "Partitioning.dat").c_str(), std::ios::binary | std::ios::app);
	outputFileStream.write((char*)&numberOfPartitions, sizeof(int));
	outputFileStream.close();

	outputFileStream.open((PreprocessingFolderStructureManager::getPartitioningFolderPath(false) + "PartitioningStatistics").c_str(), std::ios::app);
	outputFileStream << "Details about the Partitioning Iteration " << globalIterator << ":\n";
	outputFileStream << "Number of Processes = " << numberOfProcesses << "\n";
	outputFileStream << "Threshold = " << threshold << "\n";
	outputFileStream << "Number of Partitions = " << numberOfPartitions << "\n\n";

	outputFileStream.close();
}



void MasterPartitioner::testPrint(std::vector<int> *kernel, std::set<int> *border, std::set<int> *borderVisited, std::vector<int> *borderToExpand, std::set<int> *borderBalckList, std::set<int> *nextStep, std::set<int> *nextStepVisited, std::set<int> *nextStepBlackList, std::map<int, int>* mapKernelCard, std::map<int, int>* mapGlobalCard)
{
	for (int i : *kernel)
	{
		std::cout << "kernel has " << i << std::endl;
	}
	for (int i : *border)
	{
		std::cout << "border has " << i << std::endl;
	}
	for (int i : *borderVisited)
	{
		std::cout << "borderVisited has " << i << std::endl;
	}
	for (int i : *borderToExpand)
	{
		std::cout << "borderToExpand has " << i << std::endl;
	}
	for (int i : *borderBalckList)
	{
		std::cout << "borderBalckList has " << i << std::endl;
	}
	for (int i : *nextStep)
	{
		std::cout << "nextStep has " << i << std::endl;
	}
	for (int i : *nextStepVisited)
	{
		std::cout << "nextStepVisited has " << i << std::endl;
	}
	for (int i : *nextStepBlackList)
	{
		std::cout << "nextStepBlackList has " << i << std::endl;
	}
	for (std::map<int, int>::iterator it = mapKernelCard->begin(); it != mapKernelCard->end(); ++it)
	{
		std::cout << "kernelCard has " << it->first << " <-> " << it->second << std::endl;
	}
	for (std::map<int, int>::iterator it = mapGlobalCard->begin(); it != mapGlobalCard->end(); ++it)
	{
		std::cout << "globalCard has " << it->first << " <-> " << it->second << std::endl;
	}
}

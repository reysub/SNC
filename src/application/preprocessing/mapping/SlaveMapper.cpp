
#include "SlaveMapper.h"

SlaveMapper::SlaveMapper(std::map<int, std::string> *mapNodes, std::map<int, std::string> *mapPreds, std::vector<int> *preds, std::vector<int> *objs, std::map<int, std::vector<int> > *mapPos)
{
	hasToLoad = false;
	mappingId2Node = mapNodes;
	mappingId2Predicate = mapPreds;
	predicates = preds;
	objects = objs;
	mappingNode2Positions = mapPos;
}

void SlaveMapper::executeMapping(int processID, int numberOfProcesses, std::string mappingConfigurationFileURI)
{
	int operationType;

	MappingConfigurationFileManager *mcfm = MappingConfigurationFileManager::getInstance();
	mcfm->loadConfigurationFile(mappingConfigurationFileURI);

	while (true)
	{
		MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

		if (operationType == TERMINATE_MAPPING)
		{
			return;
		}

		if (operationType == CHECK_MAPPING_EXISTENCE)
		{
			checkMappingExistence(processID);
		}

		if (operationType == CREATE_MAPPING)
		{
			createMapping(processID, numberOfProcesses);
		}

		if (operationType == LOAD_MAPPING && hasToLoad)
		{
			loadMapping(processID);
		}
	}
}

void SlaveMapper::checkMappingExistence(int processID)
{
	int numberOfRequiredProcesses;
	MPI_Bcast(&numberOfRequiredProcesses, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	bool result = true;
	std::stringstream stream; stream << processID;

	if (processID < numberOfRequiredProcesses)
	{
		hasToLoad = true;

		std::ifstream inputFileStream((PreprocessingFolderStructureManager::getMappingFolderPath(true) + "MappingNodesPart" + stream.str() + ".dat").c_str(), std::ios::binary);
		result = result && inputFileStream.good();
		inputFileStream.close();

		inputFileStream.open((PreprocessingFolderStructureManager::getMappingFolderPath(true) + "MappingPredicatesPart" + stream.str() + ".dat").c_str(), std::ios::binary);
		result = result && inputFileStream.good();
		inputFileStream.close();

		inputFileStream.open((PreprocessingFolderStructureManager::getMappingFolderPath(true) + "MappingTriplesPart" + stream.str() + ".dat").c_str(), std::ios::binary);
		result = result && inputFileStream.good();
		inputFileStream.close();
	}

	int decision = (result) ? 1 : 0;
	MPI_Reduce(&decision, NULL, 1, MPI_INT, MPI_MIN, GlobalConstants::MASTER, MPI_COMM_WORLD);
}

void SlaveMapper::createMapping(int processID, int numberOfProcesses)
{
	int numberOfReceivedTriples = 0;
	std::set<int> visitedTriples;

	std::tr1::hash<std::string> hashFunction;

	char *message1Master2Slaves = (char*)malloc(GlobalConstants::MAX_CHARS_PER_LINE * sizeof(char));
	int message2All2Slave[4], message2AllToSlaveResult[4];

	std::string separationSymbol = MappingConfigurationFileManager::getInstance()->getSeparationSymbol();
	std::string subject, predicate, object, line, triple, tripleReverse;
	int designedProcess, hashSubj, hashPred, hashObj, hashTriple, hashTripleReverse;

	while (true)
	{
		MPI_Bcast(message1Master2Slaves, GlobalConstants::MAX_CHARS_PER_LINE, MPI_CHAR, GlobalConstants::MASTER, MPI_COMM_WORLD);

		line.assign(message1Master2Slaves);

		if (line.find(INPUT_FILE_OVER) != line.npos)
		{
			break;
		}

		subject = line.substr(0, line.find(separationSymbol));
		line = line.erase(0, subject.length() + separationSymbol.length());
		predicate = line.substr(0, line.find(separationSymbol));
		line = line.erase(0, predicate.length() + separationSymbol.length());
		object = line.substr(0, line.find(separationSymbol));

		triple = subject + separationSymbol + predicate + separationSymbol + object;
		tripleReverse = object + separationSymbol + predicate + separationSymbol + subject;

		hashSubj = hashFunction(subject);
		hashPred = hashFunction(predicate);
		hashObj = hashFunction(object);
		hashTriple = hashFunction(triple);
		hashTripleReverse = hashFunction(tripleReverse);
		designedProcess = ((numberOfReceivedTriples % (numberOfProcesses - 1)) + 1);

		message2All2Slave[0] = (visitedTriples.find(hashTriple) != visitedTriples.end() || visitedTriples.find(hashTripleReverse) != visitedTriples.end()) ? 1 : 0;
		message2All2Slave[1] = (mappingId2Node->find(hashSubj) != mappingId2Node->end()) ? 1 : 0;
		message2All2Slave[2] = (mappingId2Predicate->find(hashPred) != mappingId2Predicate->end()) ? 1 : 0;
		message2All2Slave[3] = (mappingId2Node->find(hashObj) != mappingId2Node->end()) ? 1 : 0;

		MPI_Reduce(message2All2Slave, message2AllToSlaveResult, 4, MPI_INT, MPI_MAX, designedProcess, MPI_COMM_WORLD);

		if (processID == designedProcess && message2AllToSlaveResult[0] == 0)
		{
			if (message2AllToSlaveResult[1] == 0)
			{
				mappingId2Node->operator [](hashSubj) = subject;
			}

			if (message2AllToSlaveResult[2] == 0)
			{
				mappingId2Predicate->operator [](hashPred) = predicate;
			}

			if (message2AllToSlaveResult[3] == 0)
			{
				mappingId2Node->operator [](hashObj) = object;
			}

			predicates->push_back(hashPred);
			objects->push_back(hashObj);
			if (mappingNode2Positions->find(hashSubj) == mappingNode2Positions->end())
			{
				mappingNode2Positions->operator [](hashSubj) = *(new std::vector<int>);
			}
			mappingNode2Positions->operator [](hashSubj).push_back(predicates->size() - 1);

			predicates->push_back(hashPred);
			objects->push_back(hashSubj);
			if (mappingNode2Positions->find(hashObj) == mappingNode2Positions->end())
			{
				mappingNode2Positions->operator [](hashObj) = *(new std::vector<int>);
			}
			mappingNode2Positions->operator [](hashObj).push_back(predicates->size() - 1);

			visitedTriples.insert(hashTriple);
		}

		numberOfReceivedTriples++;
	}

	visitedTriples.clear();
	free(message1Master2Slaves);

	int numberOfNodes = mappingId2Node->size(), numberOfPredicates = mappingId2Predicate->size(), numberOfCorrectTriples = objects->size() / 2;
	MPI_Reduce(&numberOfNodes, NULL, 1, MPI_INT, MPI_SUM, GlobalConstants::MASTER, MPI_COMM_WORLD);
	MPI_Reduce(&numberOfPredicates, NULL, 1, MPI_INT, MPI_SUM, GlobalConstants::MASTER, MPI_COMM_WORLD);
	MPI_Reduce(&numberOfCorrectTriples, NULL, 1, MPI_INT, MPI_SUM, GlobalConstants::MASTER, MPI_COMM_WORLD);

	saveMapping(processID);
}

void SlaveMapper::saveMapping(int processID)
{
	std::stringstream stream; stream << processID;
	int collectionSize, elementLength;
	std::ofstream outputFileStream;

	collectionSize = mappingId2Node->size();
	outputFileStream.open((PreprocessingFolderStructureManager::getMappingFolderPath(true) + "MappingNodesPart" + stream.str() + ".dat").c_str(), std::ios::binary);
	outputFileStream.write((char*)&collectionSize, sizeof(int));
	for (std::map<int, std::string>::iterator it = mappingId2Node->begin(); it != mappingId2Node->end(); ++it)
	{
		elementLength = it->second.length();
		outputFileStream.write((char*)&it->first, sizeof(int));
		outputFileStream.write((char*)&elementLength, sizeof(int));
		outputFileStream.write(it->second.c_str(), elementLength);
	}
	outputFileStream.close();

	collectionSize = mappingId2Predicate->size();
	outputFileStream.open((PreprocessingFolderStructureManager::getMappingFolderPath(true) + "MappingPredicatesPart" + stream.str() + ".dat").c_str(), std::ios::binary);
	outputFileStream.write((char*)&collectionSize, sizeof(int));
	for (std::map<int, std::string>::iterator it = mappingId2Predicate->begin(); it != mappingId2Predicate->end(); ++it)
	{
		elementLength = it->second.length();
		outputFileStream.write((char*)&it->first, sizeof(int));
		outputFileStream.write((char*)&elementLength, sizeof(int));
		outputFileStream.write(it->second.c_str(), elementLength);
	}
	outputFileStream.close();

	collectionSize = objects->size();
	outputFileStream.open((PreprocessingFolderStructureManager::getMappingFolderPath(true) + "MappingTriplesPart" + stream.str() + ".dat").c_str(), std::ios::binary);
	outputFileStream.write((char*)&collectionSize, sizeof(int));
	for (std::map<int, std::vector<int>>::iterator mapIT = mappingNode2Positions->begin(); mapIT != mappingNode2Positions->end(); ++mapIT)
	{
		for (std::vector<int>::iterator vecIT = mapIT->second.begin(); vecIT != mapIT->second.end(); ++vecIT)
		{
			outputFileStream.write((char*)&mapIT->first, sizeof(int));
			outputFileStream.write((char*)&(predicates->operator [](*vecIT)), sizeof(int));
			outputFileStream.write((char*)&(objects->operator [](*vecIT)), sizeof(int));
		}
	}
	outputFileStream.close();
}

void SlaveMapper::loadMapping(int processID)
{
	std::stringstream stream; stream << processID;
	int collectionSize, elementLength, elementID;
	int subject, predicate, object;
	std::ifstream inputFileStream;
	char *elementChar = NULL; std::string elementString;

	inputFileStream.open((PreprocessingFolderStructureManager::getMappingFolderPath(true) + "MappingNodesPart" + stream.str() + ".dat").c_str(), std::ios::binary);

	inputFileStream.read((char*)&collectionSize, sizeof(int));

	for (int i = 0; i < collectionSize; ++i)
	{
		inputFileStream.read((char*)&elementID, sizeof(int));
		inputFileStream.read((char*)&elementLength, sizeof(int));
		elementChar = (char*)realloc(elementChar, elementLength * sizeof(char));
		inputFileStream.read(elementChar, elementLength);
		elementString.assign(elementChar);
		elementString = elementString.substr(0, elementLength);
		mappingId2Node->operator [](elementID) = elementString;
	}
	inputFileStream.close();

	inputFileStream.open((PreprocessingFolderStructureManager::getMappingFolderPath(true) + "MappingPredicatesPart" + stream.str() + ".dat").c_str(), std::ios::binary);
	inputFileStream.read((char*)&collectionSize, sizeof(int));
	for (int i = 0; i < collectionSize; ++i)
	{
		inputFileStream.read((char*)&elementID, sizeof(int));
		inputFileStream.read((char*)&elementLength, sizeof(int));
		elementChar = (char*)realloc(elementChar, elementLength * sizeof(char));
		inputFileStream.read(elementChar, elementLength);
		elementString.assign(elementChar); elementString = elementString.substr(0, elementLength);
		mappingId2Predicate->operator [](elementID) = elementString;
	}
	inputFileStream.close();

	inputFileStream.open((PreprocessingFolderStructureManager::getMappingFolderPath(true) + "MappingTriplesPart" + stream.str() + ".dat").c_str(), std::ios::binary);
	inputFileStream.read((char*)&collectionSize, sizeof(int));
	for (int i = 0; i < collectionSize; ++i)
	{
		inputFileStream.read((char*)&subject, sizeof(int));
		inputFileStream.read((char*)&predicate, sizeof(int));
		inputFileStream.read((char*)&object, sizeof(int));

		predicates->push_back(predicate);
		objects->push_back(object);
		if (mappingNode2Positions->find(subject) == mappingNode2Positions->end())
		{
			mappingNode2Positions->operator [](subject) = *(new std::vector<int>);
		}
		mappingNode2Positions->operator [](subject).push_back(predicates->size() - 1);
	}
	inputFileStream.close();
}


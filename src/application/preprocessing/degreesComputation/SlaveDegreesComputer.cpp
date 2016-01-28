
#include "SlaveDegreesComputer.h"

SlaveDegreesComputer::SlaveDegreesComputer(std::map<int, std::string> *mapIdNode, std::map<int, std::vector<int> > *mapPos, std::map<int, int> *mapNodeCard)
{
	hasToLoad = false;
	mappingId2Node = mapIdNode;
	mappingNode2Positions = mapPos;
	mappingNode2Cardinality = mapNodeCard;
}

void SlaveDegreesComputer::executeDegreesComputation(int processID, int numberOfProcesses, std::string degreesComputationConfigurationFileURI, int globalIterator)
{
	DegreesComputationConfigurationFileManager *dcfm = DegreesComputationConfigurationFileManager::getInstance();
	dcfm->loadConfigurationFile(degreesComputationConfigurationFileURI);

	int operationType;

	while (true)
	{
		MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

		if (operationType == TERMINATE_DEGREES_COMPUTATION)
		{
			return;
		}

		if (operationType == CHECK_DEGREES_COMPUTATION_EXISTENCE)
		{
			checkDegreesComputationExistence(processID, globalIterator);
		}

		if (operationType == CREATE_DEGREES_COMPUTATION)
		{
			createDegreesComputation(processID, numberOfProcesses, globalIterator);
		}

		if (operationType == LOAD_DEGREES_COMPUTATION && hasToLoad)
		{
			loadDegreesComputation(processID, globalIterator);
		}
	}
}

void SlaveDegreesComputer::checkDegreesComputationExistence(int processID, int globalIterator)
{
	int numberOfRequiredProcesses;
	MPI_Bcast(&numberOfRequiredProcesses, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	bool result = true;
	std::stringstream processIDstream; processIDstream << processID;
	std::stringstream globalIteratorstream; globalIteratorstream << globalIterator;

	if (processID < numberOfRequiredProcesses)
	{
		hasToLoad = true;

		std::ifstream inputFileStream((PreprocessingFolderStructureManager::getDegreesComputationFolderPath(true) + "DegreesPart_" + processIDstream.str() + "_" + globalIteratorstream.str() + ".dat").c_str(), std::ios::binary);
		result = inputFileStream.good();
		inputFileStream.close();
	}

	int decision = (result) ? 1 : 0;
	MPI_Reduce(&decision, NULL, 1, MPI_INT, MPI_MIN, GlobalConstants::MASTER, MPI_COMM_WORLD);
}

void SlaveDegreesComputer::createDegreesComputation(int processID, int numberOfProcesses, int globalIterator)
{
	int numberOfNodesOfASlave, node, partialCardinality, totalCardinality;
	int maximumCardinality = 0, minimumCardinality = 999999;

	for (int pid = 1; pid < numberOfProcesses; ++pid)
	{
		numberOfNodesOfASlave = mappingId2Node->size();
		MPI_Bcast(&numberOfNodesOfASlave, 1, MPI_INT, pid, MPI_COMM_WORLD);

		if (pid == processID)
		{
			for (std::map<int, std::string>::iterator it = mappingId2Node->begin(); it != mappingId2Node->end(); ++it)
			{
				node = it->first;
				MPI_Bcast(&node, 1, MPI_INT, pid, MPI_COMM_WORLD);
				partialCardinality = (mappingNode2Positions->find(node) != mappingNode2Positions->end()) ? mappingNode2Positions->at(node).size() : 0;
				MPI_Reduce(&partialCardinality, &totalCardinality, 1, MPI_INT, MPI_SUM, pid, MPI_COMM_WORLD);
				mappingNode2Cardinality->operator [](node) = totalCardinality;
				if (totalCardinality > maximumCardinality) { maximumCardinality = totalCardinality; }
				if (totalCardinality < minimumCardinality) { minimumCardinality = totalCardinality; }
			}
		}

		else
		{
			for (int count = 0; count < numberOfNodesOfASlave; ++count)
			{
				MPI_Bcast(&node, 1, MPI_INT, pid, MPI_COMM_WORLD);
				partialCardinality = (mappingNode2Positions->find(node) != mappingNode2Positions->end()) ? mappingNode2Positions->at(node).size() : 0;
				MPI_Reduce(&partialCardinality, NULL, 1, MPI_INT, MPI_SUM, pid, MPI_COMM_WORLD);
			}
		}
	}

	MPI_Reduce(&maximumCardinality, NULL, 1, MPI_INT, MPI_MAX, GlobalConstants::MASTER, MPI_COMM_WORLD);
	MPI_Reduce(&minimumCardinality, NULL, 1, MPI_INT, MPI_MIN, GlobalConstants::MASTER, MPI_COMM_WORLD);

	saveDegreesComputation(processID, globalIterator);
}

void SlaveDegreesComputer::saveDegreesComputation(int processID, int globalIterator)
{
	std::stringstream processIDstream; processIDstream << processID;
	std::stringstream globalIteratorstream; globalIteratorstream << globalIterator;

	std::ofstream outputFileStream((PreprocessingFolderStructureManager::getDegreesComputationFolderPath(true) + "DegreesPart_" + processIDstream.str() + "_" + globalIteratorstream.str() + ".dat").c_str(), std::ios::binary);

	int collectionSize = mappingNode2Cardinality->size();
	outputFileStream.write((char*)&collectionSize, sizeof(int));

	for (std::map<int, int>::iterator it = mappingNode2Cardinality->begin(); it != mappingNode2Cardinality->end(); ++it)
	{
		outputFileStream.write((char*)&it->first, sizeof(int));
		outputFileStream.write((char*)&it->second, sizeof(int));
	}

	outputFileStream.close();
}

void SlaveDegreesComputer::loadDegreesComputation(int processID, int globalIterator)
{
	std::stringstream processIDstream; processIDstream << processID;
	std::stringstream globalIteratorstream; globalIteratorstream << globalIterator;

	std::ifstream inputFileStream((PreprocessingFolderStructureManager::getDegreesComputationFolderPath(true) + "DegreesPart_" + processIDstream.str() + "_" + globalIteratorstream.str() + ".dat").c_str(), std::ios::binary);

	int collectionSize, node, cardinality;
	inputFileStream.read((char*)&collectionSize, sizeof(int));

	for (int i = 0; i < collectionSize; ++i)
	{
		inputFileStream.read((char*)&node, sizeof(int));
		inputFileStream.read((char*)&cardinality, sizeof(int));
		mappingNode2Cardinality->operator [](node) = cardinality;
	}

	inputFileStream.close();
}

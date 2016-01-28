#include "SlaveFacadePreprocessing.h"

void SlaveFacadePreprocessing::executePreprocessing(int processID, int numberOfProcesses)
{
	int operationType, globalIterator = 0;
	SlaveMapper mapper(&mappingId2Node, &mappingId2Predicate, &predicates, &objects, &mappingNode2Positions);
	SlaveDegreesComputer degreesComputer(&mappingId2Node, &mappingNode2Positions, &mappingNode2Cardinality);
	SlavePartitioner partitioner(&mappingId2Node, &mappingId2Predicate, &predicates, &objects, &mappingNode2Positions, &mappingNode2Cardinality);
	RootConfigurationFileManager *rcfm = RootConfigurationFileManager::getInstance();


	while (true)
	{
		MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

		if (operationType == TERMINATE_PREPROCESSING)
		{
			return;
		}

		if (operationType == INITIALIZE_MAPPING)
		{
			mapper.executeMapping(processID, numberOfProcesses, rcfm->getMappingFileURI());
		}

		if (operationType == INITIALIZE_DEGREES_COMPUTATION)
		{
			degreesComputer.executeDegreesComputation(processID, numberOfProcesses, rcfm->getDegreesComputingFileURI(), globalIterator);
			globalIterator++;
		}

		if (operationType == INITIALIZE_PARTITIONING)
		{
			partitioner.executePartitioning(processID, numberOfProcesses, rcfm->getPartitioningFileURI());
		}
	}
}

std::string SlaveFacadePreprocessing::getPartitionUri(int iterationNumber, int partitionNumber)
{
	std::stringstream iterationStream, partitionStream;
	iterationStream << iterationNumber; partitionStream << partitionNumber;
	return PreprocessingFolderStructureManager::getPartitioningFolderPath(true) + "Partition_" + iterationStream.str() + "_" + partitionStream.str() + ".dat";
}

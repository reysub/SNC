
#include "MasterFacadePreprocessing.h"

bool MasterFacadePreprocessing::executePreprocessing(int numberOfProcesses, std::vector<int> *numberOfPartitions)
{
	RootConfigurationFileManager *rcfm = RootConfigurationFileManager::getInstance();
	PreprocessingFolderStructureManager::createPreprocessingFolderStructure();

	clock_t begin_time;
	MasterMapper mapper;
	MasterDegreesComputer degreesComputer;
	MasterPartitioner partitioner;
	int operationType, numberOfIterations = 0;

	std::ifstream inputFileStream((PreprocessingFolderStructureManager::getPreprocessingFolderPath() + "NumberOfIterations.dat").c_str(), std::ios::binary);

	if (inputFileStream.good())
	{
		inputFileStream.read((char*)&numberOfIterations, sizeof(int));

		if (numberOfIterations > 0 && partitioner.checkPartitioningExistence(numberOfPartitions, numberOfIterations))
		{
			operationType = TERMINATE_PREPROCESSING;
			MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
			inputFileStream.close();
			return true;
		}

		inputFileStream.close();
	}

	operationType = INITIALIZE_MAPPING;
	MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
	std::cout << "\nBegin Mapping Initialization\n";
	begin_time = clock();
	if(!mapper.executeMapping(numberOfProcesses, rcfm->getInputFileURI(), rcfm->getMappingFileURI()))
	{
		operationType = TERMINATE_PREPROCESSING;
		MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
		return false;
	}
	std::cout << "Mapping Initialization Complete (" << float( clock () - begin_time ) /  CLOCKS_PER_SEC << " secs)\n";


	int globalIterator = 0, numberOfPartitionsInCurrentIteration;

	while(true)
	{
		std::cout << "\n****************************** BEGIN OF ITERATION " << globalIterator << " ******************************\n";

		operationType = INITIALIZE_DEGREES_COMPUTATION;
		MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
		std::cout << "\nBegin Degrees Computation Initialization\n";
		begin_time = clock();
		if(!degreesComputer.executeDegreesComputation(numberOfProcesses, rcfm->getDegreesComputingFileURI(), globalIterator, numberOfIterations))
		{
			operationType = TERMINATE_PREPROCESSING;
			MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
			return false;
		}
		std::cout << "Degrees Computation Initialization Complete (" << float( clock () - begin_time ) /  CLOCKS_PER_SEC << " secs)\n";


		operationType = INITIALIZE_PARTITIONING;
		MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
		std::cout << "\nBegin Partitioning Initialization\n";
		begin_time = clock();

		numberOfPartitionsInCurrentIteration = partitioner.executePartitioning(numberOfProcesses, rcfm->getPartitioningFileURI(), globalIterator);

		if(numberOfPartitionsInCurrentIteration == 0)
		{
			operationType = TERMINATE_PREPROCESSING;
			MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
			return false;
		}

		else if(numberOfPartitionsInCurrentIteration > 0)
		{
			numberOfPartitions->push_back(numberOfPartitionsInCurrentIteration);
			std::cout << "Partitioning Initialization Complete (" << float( clock () - begin_time ) /  CLOCKS_PER_SEC << " secs)\n";
		}

		else
		{
			numberOfPartitions->push_back(-1 * numberOfPartitionsInCurrentIteration);
			std::cout << "Partitioning Initialization Complete (" << float( clock () - begin_time ) /  CLOCKS_PER_SEC << " secs)\n";
			break;
		}

		globalIterator++;
	}

	int collectionSize = numberOfPartitions->size();
	std::ofstream outputFileStream ((PreprocessingFolderStructureManager::getPreprocessingFolderPath() + "NumberOfIterations.dat").c_str(), std::ios::binary);
	outputFileStream.write((char*)&collectionSize, sizeof(int));
	outputFileStream.close();

	std::cout << "\n****************************** PREPROCESSING PHASE COMPLETE ******************************\n\n";

	operationType = TERMINATE_PREPROCESSING;
	MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
	return true;
}


#include "MasterDegreesComputer.h"

bool MasterDegreesComputer::executeDegreesComputation(int numberOfProcesses, std::string degreesComputationConfigurationFileURI, int globalIterator, int numberOfIterations)
{
	DegreesComputationConfigurationFileManager *dcfm = DegreesComputationConfigurationFileManager::getInstance();

	int numberOfRequiredProcesses;

	if (!dcfm->loadConfigurationFile(degreesComputationConfigurationFileURI))
	{
		terminateDegreesComputation();
		return false;
	}

	std::stringstream globalIteratorStream; globalIteratorStream << globalIterator;
	std::ifstream inputFileStream((PreprocessingFolderStructureManager::getDegreesComputationFolderPath(true) + "DegreesComputation.dat").c_str(), std::ios::binary);

	if (!inputFileStream.good() || globalIterator >= numberOfIterations)
	{
		inputFileStream.close();
		createDegreesComputation(numberOfProcesses, globalIterator);
		terminateDegreesComputation();
		return true;
	}

	inputFileStream.seekg(globalIterator * sizeof(int), std::ios::beg);
	inputFileStream.read((char*)&numberOfRequiredProcesses, sizeof(int));

	if (numberOfRequiredProcesses > numberOfProcesses)
	{
		std::cout << "\nError: at least " << numberOfRequiredProcesses << " are required for attempting to load the degrees computation\n\n";
		inputFileStream.close();
		terminateDegreesComputation();
		return false;
	}

	if (numberOfRequiredProcesses < numberOfProcesses)
	{
		std::cout << "Warning: only " << numberOfRequiredProcesses << " processes are required, but " << numberOfProcesses << " were provided instead\n";
	}

	if (checkDegreesComputationExistence(numberOfRequiredProcesses))
	{
		loadDegreesComputation();
	} else {
		createDegreesComputation(numberOfProcesses, globalIterator);
	}

	terminateDegreesComputation();
	return true;
}

bool MasterDegreesComputer::checkDegreesComputationExistence(int numberOfRequiredProcesses)
{
	int operationType = CHECK_DEGREES_COMPUTATION_EXISTENCE;
	MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
	MPI_Bcast(&numberOfRequiredProcesses, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	int result = 1, totalResult;
	MPI_Reduce(&result, &totalResult, 1, MPI_INT, MPI_MIN, GlobalConstants::MASTER, MPI_COMM_WORLD);

	return (totalResult == 1);
}

void MasterDegreesComputer::createDegreesComputation(int numberOfProcesses, int globalIterator)
{
	std::cout << "	Begin Creating Degrees Computation\n";

	int operationType = CREATE_DEGREES_COMPUTATION;
	MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	int numberOfNodesOfASlave, node, zero = 0, numberOfparsedNodes = 0;
	int alertFrequency = DegreesComputationConfigurationFileManager::getInstance()->getAlertFrequency();

	for (int pid = 1; pid < numberOfProcesses; ++pid)
	{
		MPI_Bcast(&numberOfNodesOfASlave, 1, MPI_INT, pid, MPI_COMM_WORLD);

		for (int count = 0; count < numberOfNodesOfASlave; ++count)
		{
			MPI_Bcast(&node, 1, MPI_INT, pid, MPI_COMM_WORLD);
			MPI_Reduce(&zero, NULL, 1, MPI_INT, MPI_SUM, pid, MPI_COMM_WORLD);

			numberOfparsedNodes++;
			if (numberOfparsedNodes % alertFrequency == 0)
			{
				std::cout << "		parsed node " << numberOfparsedNodes << "\n";
			}
		}
	}

	int maximumCardinality, minimumCardinality, infinity = 999999;
	MPI_Reduce(&zero, &maximumCardinality, 1, MPI_INT, MPI_MAX, GlobalConstants::MASTER, MPI_COMM_WORLD);
	MPI_Reduce(&infinity, &minimumCardinality, 1, MPI_INT, MPI_MIN, GlobalConstants::MASTER, MPI_COMM_WORLD);

	std::cout << "	Degrees Computation Creation Complete\n";

	saveDegreesComputation(numberOfProcesses, maximumCardinality, minimumCardinality, globalIterator);
}

void MasterDegreesComputer::saveDegreesComputation(int numberOfProcesses, int maxCard, int minCard, int globalIterator)
{
	std::cout << "	Begin Saving Degrees Computation\n";

	std::stringstream globalIteratorStream; globalIteratorStream << globalIterator;

	std::ofstream outputFileStream((PreprocessingFolderStructureManager::getDegreesComputationFolderPath(false) + "DegreesComputationStatistics").c_str(), std::ios::app);
	outputFileStream << "Details about the Degrees Computation Iteration " << globalIterator << "\n";
	outputFileStream << "Number of Processes = " << numberOfProcesses << "\n";
	outputFileStream << "Maximum Cardinality = " << maxCard << "\n";
	outputFileStream << "Minimum Cardinality = " << minCard << "\n\n";
	outputFileStream.close();

	outputFileStream.open((PreprocessingFolderStructureManager::getDegreesComputationFolderPath(true) + "DegreesComputation.dat").c_str(), std::ios::binary | std::ios::app);
	outputFileStream.write((char*)&numberOfProcesses, sizeof(int));
	outputFileStream.close();

	std::cout << "	Degrees Computation Saving Complete\n";
}

void MasterDegreesComputer::loadDegreesComputation()
{
	std::cout << "	Begin Loading Degrees Computation\n";
	int operationType = LOAD_DEGREES_COMPUTATION;
	MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
	std::cout << "	Degrees Computation Loading Complete\n";
}

void MasterDegreesComputer::terminateDegreesComputation()
{
	int operationType = TERMINATE_DEGREES_COMPUTATION;
	MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
}

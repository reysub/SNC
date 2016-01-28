#include "application/preprocessing/facadePreprocessing/MasterFacadePreprocessing.h"
#include "application/preprocessing/facadePreprocessing/SlaveFacadePreprocessing.h"

void executeMaster(int);
void executeSlave(int, int);

int main(int argc, char **argv)
{
	int processID, numberOfProcesses;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &processID);
	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);

	if (numberOfProcesses < 2)
	{
		std::cout << "\nError, please run this application with at least 2 processes\n\n";
		MPI_Finalize();
		return 1;
	}

	if (argc != 2 && processID == GlobalConstants::MASTER)
	{
		std::cout << "\nError, please provide the root configuration file URI\n\n";
		MPI_Finalize();
		return 1;
	}

	if (!RootConfigurationFileManager::getInstance()->loadConfigurationFile(argv[1], processID))
	{
		MPI_Finalize();
		return 1;
	}

	if (processID == GlobalConstants::MASTER)
	{
		executeMaster(numberOfProcesses);
	} else {
		executeSlave(processID, numberOfProcesses);
	}

	std::cout << "Process " << processID << " has completed its execution\n";

	MPI_Finalize();
	return 1;
}

void executeMaster(int numberOfProcesses)
{
	std::vector<int> numberOfPartitions;

	MasterFacadePreprocessing masterFacadePreprocessing;
	if (!masterFacadePreprocessing.executePreprocessing(numberOfProcesses, &numberOfPartitions))
	{
		return;
	}
}

void executeSlave(int processID, int numberOfProcesses)
{
	SlaveFacadePreprocessing slaveFacadePreprocessing;
	slaveFacadePreprocessing.executePreprocessing(processID, numberOfProcesses);
}


#ifndef DEGREESCOMPUTER_H_
#define DEGREESCOMPUTER_H_

#include <mpi.h>	// mpi context
#include "../../../configuration/fileManager/preprocessing/DegreesComputationConfigurationFileManager.h"
#include "../../../configuration/folderManager/PreprocessingFolderStructureManager.h"

class DegreesComputer {

protected:
	int TERMINATE_DEGREES_COMPUTATION = 1;
	int CHECK_DEGREES_COMPUTATION_EXISTENCE = 2;
	int CREATE_DEGREES_COMPUTATION = 3;
	int LOAD_DEGREES_COMPUTATION = 4;
};

#endif

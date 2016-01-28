
#ifndef MAPPER_H_
#define MAPPER_H_

#include <mpi.h>		// mpi context
#include "../../../configuration/fileManager/preprocessing/MappingConfigurationFileManager.h"
#include "../../../configuration/folderManager/PreprocessingFolderStructureManager.h"

class Mapper {

protected:
	int TERMINATE_MAPPING = 1;
	int CHECK_MAPPING_EXISTENCE = 2;
	int CREATE_MAPPING = 3;
	int LOAD_MAPPING = 4;

	std::string INPUT_FILE_OVER = "__INPUT_FILE_OVER_SIGNAL__";
};

#endif

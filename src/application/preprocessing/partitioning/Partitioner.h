
#ifndef PARTITIONER_H_
#define PARTITIONER_H_

#include <mpi.h>	// mpi context
#include <vector>	// vectors
#include <set>		// sets
#include <map>		// maps
#include "../../../configuration/fileManager/preprocessing/PartitioningConfigurationFileManager.h"
#include "../../../configuration/folderManager/PreprocessingFolderStructureManager.h"

class Partitioner {

protected:
	int TERMINATE_PARTITIONING = 1;
	int CREATE_PARTITIONING = 2;

	int NORMAL_EXECUTION = 21;
	int GET_SUGGESTION = 22;
	int ADD_ALL_TO_VISITED = 23;

	int RETRIEVE_TEXTUAL_REPRESENTATION = 24;

	std::string TRANSLATION_SEPARATION_SYMBOL = "(-)";
	std::string MISSING_TRANSLATION_SYMBOL = "?MISS?";

};

#endif

#ifndef CONSTANTMANAGER_H_
#define CONSTANTMANAGER_H_

#include <string>


namespace GlobalConstants
{
	const int MASTER 					= 0;
	const int MAX_CHARS_PER_LINE 		= 4096;
	const std::string EXECUTION_FOLDER 	= "_Execution/";
}


namespace ConfigurationConstants
{
	const std::string FIELD_SEPARATION_SYMBOL 	= "=";
	const std::string VALUES_SEPARATION_SYMBOL 	= ":";
	const std::string COMMENT_STARTING_SYMBOL 	= "#";
}


namespace PreprocessingFolderConstants
{
	const std::string PREPROCESSING_ROOT_FOLDER 	= "Preprocessing/";
	const std::string MAPPING_FOLDER_1 				= "Mapping/";
	const std::string MAPPING_FOLDER_2 				= "Mapping_Data/";
	const std::string DEGREES_COMPUTATION_FOLDER_1 	= "DegreesComputation/";
	const std::string DEGREES_COMPUTATION_FOLDER_2 	= "DegreesComputation_Data/";
	const std::string PARTITIONING_FOLDER_1 		= "Partitioning/";
	const std::string PARTITIONING_FOLDER_2 		= "Partitioning_Data/";
}


#endif

#include "PreprocessingFolderStructureManager.h"

void PreprocessingFolderStructureManager::createPreprocessingFolderStructure()
{
	std::string inputFileURI = RootConfigurationFileManager::getInstance()->getInputFileURI();
	mkdir((inputFileURI + GlobalConstants::EXECUTION_FOLDER).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	mkdir((inputFileURI + GlobalConstants::EXECUTION_FOLDER + PreprocessingFolderConstants::PREPROCESSING_ROOT_FOLDER).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	mkdir((inputFileURI + GlobalConstants::EXECUTION_FOLDER + PreprocessingFolderConstants::PREPROCESSING_ROOT_FOLDER + PreprocessingFolderConstants::MAPPING_FOLDER_1).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir((inputFileURI + GlobalConstants::EXECUTION_FOLDER + PreprocessingFolderConstants::PREPROCESSING_ROOT_FOLDER + PreprocessingFolderConstants::MAPPING_FOLDER_1 + PreprocessingFolderConstants::MAPPING_FOLDER_2).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	mkdir((inputFileURI + GlobalConstants::EXECUTION_FOLDER + PreprocessingFolderConstants::PREPROCESSING_ROOT_FOLDER + PreprocessingFolderConstants::DEGREES_COMPUTATION_FOLDER_1).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir((inputFileURI + GlobalConstants::EXECUTION_FOLDER + PreprocessingFolderConstants::PREPROCESSING_ROOT_FOLDER + PreprocessingFolderConstants::DEGREES_COMPUTATION_FOLDER_1 + PreprocessingFolderConstants::DEGREES_COMPUTATION_FOLDER_2).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	mkdir((inputFileURI + GlobalConstants::EXECUTION_FOLDER + PreprocessingFolderConstants::PREPROCESSING_ROOT_FOLDER + PreprocessingFolderConstants::PARTITIONING_FOLDER_1).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir((inputFileURI + GlobalConstants::EXECUTION_FOLDER + PreprocessingFolderConstants::PREPROCESSING_ROOT_FOLDER + PreprocessingFolderConstants::PARTITIONING_FOLDER_1 + PreprocessingFolderConstants::PARTITIONING_FOLDER_2).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

std::string PreprocessingFolderStructureManager::getPreprocessingFolderPath()
{
	return RootConfigurationFileManager::getInstance()->getInputFileURI() + GlobalConstants::EXECUTION_FOLDER + PreprocessingFolderConstants::PREPROCESSING_ROOT_FOLDER;
}

std::string PreprocessingFolderStructureManager::getMappingFolderPath(bool innerFolder)
{
	std::string path = RootConfigurationFileManager::getInstance()->getInputFileURI() + GlobalConstants::EXECUTION_FOLDER + PreprocessingFolderConstants::PREPROCESSING_ROOT_FOLDER + PreprocessingFolderConstants::MAPPING_FOLDER_1;
	if (innerFolder)
	{
		path += PreprocessingFolderConstants::MAPPING_FOLDER_2;
	}

	return path;
}

std::string PreprocessingFolderStructureManager::getDegreesComputationFolderPath(bool innerFolder)
{
	std::string path = RootConfigurationFileManager::getInstance()->getInputFileURI() + GlobalConstants::EXECUTION_FOLDER + PreprocessingFolderConstants::PREPROCESSING_ROOT_FOLDER + PreprocessingFolderConstants::DEGREES_COMPUTATION_FOLDER_1;
	if (innerFolder)
	{
		path += PreprocessingFolderConstants::DEGREES_COMPUTATION_FOLDER_2;
	}

	return path;
}

std::string PreprocessingFolderStructureManager::getPartitioningFolderPath(bool innerFolder)
{
	std::string path = RootConfigurationFileManager::getInstance()->getInputFileURI() + GlobalConstants::EXECUTION_FOLDER + PreprocessingFolderConstants::PREPROCESSING_ROOT_FOLDER + PreprocessingFolderConstants::PARTITIONING_FOLDER_1;
	if (innerFolder)
	{
		path += PreprocessingFolderConstants::PARTITIONING_FOLDER_2;
	}

	return path;
}

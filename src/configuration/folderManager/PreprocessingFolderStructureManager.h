#ifndef PREPROCESSINGFOLDERSTRUCTUREMANAGER_H_
#define PREPROCESSINGFOLDERSTRUCTUREMANAGER_H_

#include <sys/stat.h>
#include "../constants/ConstantsManager.h"
#include "../fileManager/RootConfigurationFileManager.h"

class PreprocessingFolderStructureManager {

	public:
		static void createPreprocessingFolderStructure();

		static std::string getPreprocessingFolderPath();
		static std::string getMappingFolderPath( bool );
		static std::string getDegreesComputationFolderPath( bool );
		static std::string getPartitioningFolderPath( bool );
};

#endif

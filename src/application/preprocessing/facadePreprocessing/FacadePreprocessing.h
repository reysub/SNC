#ifndef FACADEPREPROCESSING_H_
#define FACADEPREPROCESSING_H_

#include "../../../configuration/fileManager/RootConfigurationFileManager.h"

class FacadePreprocessing {

protected:
	int TERMINATE_PREPROCESSING = 1;
	int INITIALIZE_MAPPING = 2;
	int INITIALIZE_DEGREES_COMPUTATION = 3;
	int INITIALIZE_PARTITIONING = 4;
};

#endif

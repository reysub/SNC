#ifndef ROOTCONFIGURATIONFILEMANAGER_H_
#define ROOTCONFIGURATIONFILEMANAGER_H_

#include <cstdio>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include "../constants/ConstantsManager.h"
#include "../../application/preprocessing/FileDownloader.h"


class RootConfigurationFileManager {

	private:
		static RootConfigurationFileManager *instance;
		std::string inputFileURI;
		std::string mappingFileURI;
		std::string degreesComputingFileURI;
		std::string partitioningFileURI;

		RootConfigurationFileManager();
		bool checkParameters( int );


	public:
		static RootConfigurationFileManager* getInstance();

		bool loadConfigurationFile ( std::string, int );

		std::string getInputFileURI();
		std::string getMappingFileURI();
		std::string getDegreesComputingFileURI();
		std::string getPartitioningFileURI();
};

#endif

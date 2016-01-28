
#ifndef PARTITIONINGCONFIGURATIONFILEMANAGER_H_
#define PARTITIONINGCONFIGURATIONFILEMANAGER_H_

#include <cstdio>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include "../../constants/ConstantsManager.h"

class PartitioningConfigurationFileManager {

private:
	static PartitioningConfigurationFileManager *instance;
	int alertFrequency;
	int maximumThreshold;
	int minimumThreshold;
	double expansionRatio;
	double expansionPercentage;
	bool respectExpansionRatio;
	bool respectExpansionPercentage;
	bool respectMaximumThreshold;

	PartitioningConfigurationFileManager();
	bool checkParameters(std::string, std::string, std::string, std::string, std::string, std::string);

public:
	static PartitioningConfigurationFileManager* getInstance();
	bool loadConfigurationFile (std::string);
	int getAlertFrequency();
	int getMaximumThreshold();
	int getMinimumThreshold();
	double getexpansionRatio();
	double getexpansionPercentage();
	bool isToRespectExpansionRatio();
	bool isToRespectExpansionPercentage();
	bool isToRespectMaximumThreshold();
};

#endif

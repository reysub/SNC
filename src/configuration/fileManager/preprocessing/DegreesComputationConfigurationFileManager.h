
#ifndef DEGREESCOMPUTATIONCONFIGURATIONFILEMANAGER_H_
#define DEGREESCOMPUTATIONCONFIGURATIONFILEMANAGER_H_

#include <cstdio>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include "../../constants/ConstantsManager.h"

class DegreesComputationConfigurationFileManager {

private:
	static DegreesComputationConfigurationFileManager *instance;
	int alertFrequency;
	int stepSize;

	DegreesComputationConfigurationFileManager();
	bool checkParameters(std::string, std::string);

public:
	static DegreesComputationConfigurationFileManager* getInstance();

	bool loadConfigurationFile (std::string);

	int getAlertFrequency();
	int getStepSize();
};

#endif

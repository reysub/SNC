#ifndef MAPPINGCONFIGURATIONFILEMANAGER_H_
#define MAPPINGCONFIGURATIONFILEMANAGER_H_

#include <cstdio>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <set>
#include "../../constants/ConstantsManager.h"

class MappingConfigurationFileManager {

private:
	static MappingConfigurationFileManager *instance;
	int alertFrequency;
	std::string separationSymbol;
	std::set<std::string> unwantedNodes;
	std::set<std::string> unwantedEdges;
	bool excludeOnPartialMatch;


	MappingConfigurationFileManager();
	bool checkParameters( std::string );

	void parseUnwantedElements( std::ifstream*, std::set<std::string>*, std::string );

public:
	static MappingConfigurationFileManager* getInstance();

	bool loadConfigurationFile ( std::string );

	int getAlertFrequency();
	std::string getSeparationSymbol();
	std::set<std::string> *getUnwantedNodes();
	std::set<std::string> *getUnwantedEdges();
	bool isToExcludeOnPartialMatch();
};

#endif

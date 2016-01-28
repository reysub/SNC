#include "MappingConfigurationFileManager.h"

MappingConfigurationFileManager * MappingConfigurationFileManager::instance = NULL;


MappingConfigurationFileManager::MappingConfigurationFileManager()
{
	this->alertFrequency		= 0;
	this->separationSymbol 		= "";
	this->excludeOnPartialMatch = false;
}


MappingConfigurationFileManager* MappingConfigurationFileManager::getInstance()
{
	if (instance == NULL)
	{
		instance = new MappingConfigurationFileManager();
	}

	return instance;
}


bool MappingConfigurationFileManager::loadConfigurationFile( std::string configurationFileURI )
{
	std::ifstream inputFileStream( configurationFileURI.c_str() );
	if ( !inputFileStream.good() )
	{
		std::cout << "\nAn error occurred while attempting to open the mapping configuration file.\n\n";
		inputFileStream.close();
		return false;
	}

	std::string key, value, line, alertFrequencyString;
	char *buffer = ( char* ) malloc( GlobalConstants::MAX_CHARS_PER_LINE * sizeof( char ) );

	while ( !inputFileStream.eof() )
	{
		inputFileStream.getline( buffer, GlobalConstants::MAX_CHARS_PER_LINE );

		if ( buffer[0] == '\0' )
		{
			continue;
		}

		line.assign( buffer );

		if ( line.find_first_of( ConfigurationConstants::FIELD_SEPARATION_SYMBOL) == line.npos || line.find_first_of( ConfigurationConstants::COMMENT_STARTING_SYMBOL ) == 0 )
		{
			continue;
		}

		key = line.substr(0, line.find_first_of(ConfigurationConstants::FIELD_SEPARATION_SYMBOL));
		value = line.substr(line.find_first_of(ConfigurationConstants::FIELD_SEPARATION_SYMBOL) + 1, line.npos);

		if (key == "alert_frequency")
		{
			alertFrequencyString = value;
		}

		if (key == "separation_symbol")
		{
			separationSymbol = value;
		}

		if (key == "unwanted_nodes" && value == "begin")
		{
			parseUnwantedElements(&inputFileStream, &unwantedNodes, "end_unwanted_nodes");
		}

		if (key == "unwanted_edges" && value == "begin")
		{
			parseUnwantedElements(&inputFileStream, &unwantedEdges, "end_unwanted_edges");
		}

		if (key == "exclusion_criteria")
		{
			excludeOnPartialMatch = value == "partial";
		}
	}

	inputFileStream.close();

	return checkParameters(alertFrequencyString);
}


void MappingConfigurationFileManager::parseUnwantedElements(std::ifstream *inputFileStream, std::set<std::string> *set, std::string terminationSymbol)
{
	std::string line, unwantedElementsString = "";
	char* buffer = (char*)malloc(GlobalConstants::MAX_CHARS_PER_LINE * sizeof(char));


	while (!inputFileStream->eof())
	{
		inputFileStream->getline(buffer, GlobalConstants::MAX_CHARS_PER_LINE);

		if (buffer[0] == '\0')
		{
			continue;
		}

		line.assign(buffer);

		if (line.find_first_of(ConfigurationConstants::COMMENT_STARTING_SYMBOL) == 0)
		{
			continue;
		}

		if (line == terminationSymbol)
		{
			break;
		}

		set->insert(line);
	}

	free(buffer);
}


bool MappingConfigurationFileManager::checkParameters(std::string alertFrequencyString)
{
	if (alertFrequencyString.empty()) {
		std::cout << "\nError: the alert frequency is missing in the mapping configuration file\n\n";
		return false;
	}

	if (separationSymbol.empty()) {
		std::cout << "\nError: the separation symbol is missing in the mapping configuration file\n\n";
		return false;
	}

	alertFrequency = atoi(alertFrequencyString.c_str());

	if (alertFrequency < 1)
	{
		std::cout << "\nError: the alert frequency must be an integer > 0 in the mapping configuration file\n\n";
		return false;
	}

	if (separationSymbol == "space")
	{
		separationSymbol = " ";
	}

	return true;
}


int MappingConfigurationFileManager::getAlertFrequency()
{
	return alertFrequency;
}

std::string MappingConfigurationFileManager::getSeparationSymbol()
{
	return separationSymbol;
}

std::set<std::string> *MappingConfigurationFileManager::getUnwantedNodes()
{
	return &unwantedNodes;
}

std::set<std::string> *MappingConfigurationFileManager::getUnwantedEdges()
{
	return &unwantedEdges;
}

bool MappingConfigurationFileManager::isToExcludeOnPartialMatch()
{
	return excludeOnPartialMatch;
}

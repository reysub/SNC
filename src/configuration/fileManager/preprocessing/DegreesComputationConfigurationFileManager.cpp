
#include "DegreesComputationConfigurationFileManager.h"

DegreesComputationConfigurationFileManager * DegreesComputationConfigurationFileManager::instance = NULL;

DegreesComputationConfigurationFileManager::DegreesComputationConfigurationFileManager()
{
	alertFrequency = 0;
	stepSize = 0;
}

DegreesComputationConfigurationFileManager* DegreesComputationConfigurationFileManager::getInstance()
{
	if ( instance == NULL )
	{
		instance = new DegreesComputationConfigurationFileManager();
	}

	return instance;
}

bool DegreesComputationConfigurationFileManager::loadConfigurationFile( std::string configurationFileURI )
{
	std::ifstream inputFileStream( configurationFileURI.c_str() );
	if ( !inputFileStream.good() )
	{
		std::cout << "\nAn error occurred while attempting to open the degrees computation configuration file.\n\n";
		inputFileStream.close();
		return false;
	}

	std::string key, value, line, alertFrequencyString, stepSizeString;
	char* buffer = ( char* )malloc( GlobalConstants::MAX_CHARS_PER_LINE * sizeof( char ) );

	while ( !inputFileStream.eof() )
	{
		inputFileStream.getline( buffer, GlobalConstants::MAX_CHARS_PER_LINE );

		if ( buffer[0] == '\0' )
		{
			continue;
		}

		line.assign( buffer );

		if ( line.find_first_of( ConfigurationConstants::FIELD_SEPARATION_SYMBOL ) == line.npos || line.find_first_of( ConfigurationConstants::COMMENT_STARTING_SYMBOL ) == 0 )
		{
			continue;
		}

		key = line.substr( 0, line.find_first_of( ConfigurationConstants::FIELD_SEPARATION_SYMBOL ) );
		value = line.substr( line.find_first_of( ConfigurationConstants::FIELD_SEPARATION_SYMBOL ) + 1, line.npos );

		if ( key == "alert_frequency" )
		{
			alertFrequencyString = value;
		}

		if ( key == "step_size" )
		{
			stepSizeString = value;
		}
	}

	inputFileStream.close();
	return checkParameters( alertFrequencyString, stepSizeString );
}

bool DegreesComputationConfigurationFileManager::checkParameters( std::string alertFrequencyString, std::string stepSizeString )
{
	if ( alertFrequencyString.empty() ) {
		std::cout << "\nError: the alert frequency is missing in the degrees computation configuration file\n\n";
		return false;
	}

	if ( stepSizeString.empty() ) {
		std::cout << "\nError: the step size is missing in the degrees computation configuration file\n\n";
		return false;
	}

	alertFrequency = atoi( alertFrequencyString.c_str() );
	stepSize = atoi( stepSizeString.c_str() );

	if ( alertFrequency < 1 )
	{
		std::cout << "\nError: the alert frequency must be an integer > 0 in the degrees computation configuration file\n\n";
		return false;
	}

	if ( stepSize < 1 )
	{
		std::cout << "\nError: the step size must be an integer > 0 in the degrees computation configuration file\n\n";
		return false;
	}

	return true;
}

int DegreesComputationConfigurationFileManager::getAlertFrequency()
{
	return alertFrequency;
}

int DegreesComputationConfigurationFileManager::getStepSize()
{
	return stepSize;
}

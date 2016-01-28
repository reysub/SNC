
#include "RootConfigurationFileManager.h"

RootConfigurationFileManager * RootConfigurationFileManager::instance = NULL;

RootConfigurationFileManager::RootConfigurationFileManager()
{
	this->inputFileURI				= "";
	this->mappingFileURI			= "";
	this->degreesComputingFileURI	= "";
	this->partitioningFileURI		= "";
}


RootConfigurationFileManager* RootConfigurationFileManager::getInstance()
{
	if ( instance == NULL )
	{
		instance = new RootConfigurationFileManager();
	}

	return instance;
}


bool RootConfigurationFileManager::loadConfigurationFile( std::string configurationFileURI, int processID )
{
	std::ifstream inputFileStream( configurationFileURI.c_str() );
	if ( !inputFileStream.good() )
	{
		std::cout << "\nAn error occurred while attempting to open the root configuration file.\n\n";
		inputFileStream.close();
		return false;
	}

	std::string key, value, line;
	char *buffer = ( char* ) malloc( GlobalConstants::MAX_CHARS_PER_LINE * sizeof( char ) );

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
		value = line.substr(line.find_first_of( ConfigurationConstants::FIELD_SEPARATION_SYMBOL) + 1, line.npos );

		if ( key == "input_file" )
		{
			this->inputFileURI = value;
		}

		if ( key == "mapping" )
		{
			this->mappingFileURI = value;
		}

		if ( key == "degrees" )
		{
			this->degreesComputingFileURI = value;
		}

		if ( key == "partitioning" )
		{
			this->partitioningFileURI = value;
		}
	}

	inputFileStream.close();
	return this->checkParameters( processID );
}

bool RootConfigurationFileManager::checkParameters( int processID )
{
	if ( this->inputFileURI.empty() )
	{
		std::cout << "\nError: the input file uri is missing in the root configuration file\n\n";
		return false;
	}

	if ( this->mappingFileURI.empty() )
	{
		std::cout << "\nError: the mapping configuration file uri is missing in the root configuration file\n\n";
		return false;
	}

	if ( this->degreesComputingFileURI.empty() )
	{
		std::cout << "\nError: the degrees comptation configuration file uri is missing in the root configuration file\n\n";
		return false;
	}

	if ( this->partitioningFileURI.empty() )
	{
		std::cout << "\nError: the partitioning configuration file uri is missing in the root configuration file\n\n";
		return false;
	}

	return true;
}


std::string RootConfigurationFileManager::getInputFileURI()
{
	return this->inputFileURI;
}


std::string RootConfigurationFileManager::getMappingFileURI()
{
	return this->mappingFileURI;
}


std::string RootConfigurationFileManager::getDegreesComputingFileURI()
{
	return this->degreesComputingFileURI;
}


std::string RootConfigurationFileManager::getPartitioningFileURI()
{
	return this->partitioningFileURI;
}

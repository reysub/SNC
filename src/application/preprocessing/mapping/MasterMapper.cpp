
#include "MasterMapper.h"

bool MasterMapper::executeMapping(int numberOfprocesses, std::string inputFileURI, std::string mappingConfigurationFileURI)
{
	MappingConfigurationFileManager *mcfm = MappingConfigurationFileManager::getInstance();

	bool result;
	int numberOfRequiredProcesses;

	if (!mcfm->loadConfigurationFile(mappingConfigurationFileURI))
	{
		terminateMapping();
		return false;
	}

	std::ifstream inputFileStream((PreprocessingFolderStructureManager::getMappingFolderPath(true) + "Mapping.dat").c_str(), std::ios::binary);

	if (!inputFileStream.good())
	{
		inputFileStream.close();
		result = createMapping(inputFileURI, numberOfprocesses);
		terminateMapping();
		return result;
	}

	inputFileStream.read((char*)&numberOfRequiredProcesses, sizeof(int));

	if (numberOfRequiredProcesses > numberOfprocesses)
	{
		std::cout << "\nError: at least " << numberOfRequiredProcesses << " are required for attempting to load the mapping\n\n";
		inputFileStream.close();
		terminateMapping();
		return 0;
	}

	if (numberOfRequiredProcesses < numberOfprocesses)
	{
		std::cout << "Warning: only " << numberOfRequiredProcesses << " processes are required, but " << numberOfprocesses << " were provided instead\n";
	}

	if (checkMappingExistence(numberOfRequiredProcesses))
	{
		loadMapping();
		result = true;
	} else {
		result = createMapping(inputFileURI, numberOfprocesses);
	}

	terminateMapping();
	return result;
}

bool MasterMapper::checkMappingExistence(int numberOfRequiredProcesses)
{
	int operationType = CHECK_MAPPING_EXISTENCE;
	MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
	MPI_Bcast(&numberOfRequiredProcesses, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	int result = 1, totalResult;
	MPI_Reduce(&result, &totalResult, 1, MPI_INT, MPI_MIN, GlobalConstants::MASTER, MPI_COMM_WORLD);

	return (totalResult == 1);
}

bool MasterMapper::createMapping(std::string inputFileURI, int numberOfprocesses)
{
	std::cout << "	Creating Mapping\n";

	std::ifstream inputFileStream(inputFileURI.c_str());
	if (!inputFileStream.good())
	{
		std::cout << "\nAn error occurred while attempting to open the input file\n\n";
		inputFileStream.close();
		return false;
	}

	int operationType = CREATE_MAPPING;
	MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);

	std::string triple, subject, predicate, object;
	int numberOfTriples = 0, numberOfSentTriples = 0, numberOfMalformedTriples = 0, numberOfEmptyTriples = 0, numberOfFilteredTriples = 0;
	int alertFrequency = MappingConfigurationFileManager::getInstance()->getAlertFrequency();
	std::string separationSymbol = MappingConfigurationFileManager::getInstance()->getSeparationSymbol();
	bool filterSetsAreEmpty = MappingConfigurationFileManager::getInstance()->getUnwantedNodes()->empty() && MappingConfigurationFileManager::getInstance()->getUnwantedEdges()->empty();
	char *buffer = (char*) malloc(GlobalConstants::MAX_CHARS_PER_LINE * sizeof(char));
	int message2All2Slave[] = {0, 0, 0, 0};

	while (!inputFileStream.eof())
	{
		inputFileStream.getline(buffer, GlobalConstants::MAX_CHARS_PER_LINE);

		numberOfTriples++;
		if (numberOfTriples % alertFrequency == 0)
		{
			std::cout << "		Parsing line " << numberOfTriples << "\n";
		}

		if (buffer[0] == '\0')
		{
			numberOfEmptyTriples++;
			continue;
		}

		triple.assign(buffer);

		if (triple.find(separationSymbol, triple.find(separationSymbol) + separationSymbol.length()) == triple.npos)
		{
			numberOfMalformedTriples++;
			continue;
		}

		subject = triple.substr(0, triple.find(separationSymbol));
		triple = triple.erase(0, subject.length() + separationSymbol.length());
		predicate = triple.substr(0, triple.find(separationSymbol));
		triple = triple.erase(0, predicate.length() + separationSymbol.length());
		object = triple.substr(0, triple.find(separationSymbol));

		if (subject.empty() || predicate.empty() || object.empty())
		{
			numberOfMalformedTriples++;
			continue;
		}

		if (!filterSetsAreEmpty && isToFilter(subject, predicate, object))
		{
			numberOfFilteredTriples++;
			continue;
		}

		MPI_Bcast(buffer, GlobalConstants::MAX_CHARS_PER_LINE, MPI_CHAR, GlobalConstants::MASTER, MPI_COMM_WORLD);

		MPI_Reduce(message2All2Slave, NULL, 4, MPI_INT, MPI_MAX, ((numberOfSentTriples % (numberOfprocesses - 1)) + 1), MPI_COMM_WORLD);

		numberOfSentTriples++;
	}

	strncpy(buffer, INPUT_FILE_OVER.c_str(), INPUT_FILE_OVER.length());
	MPI_Bcast(buffer, GlobalConstants::MAX_CHARS_PER_LINE, MPI_CHAR, GlobalConstants::MASTER, MPI_COMM_WORLD);

	free(buffer);


	int zero = 0, numberOfNodes, numberOfPredicates, numberOfCorrectTriples, numberOfDuplicatedTriples;
	MPI_Reduce(&zero, &numberOfNodes, 1, MPI_INT, MPI_SUM, GlobalConstants::MASTER, MPI_COMM_WORLD);
	MPI_Reduce(&zero, &numberOfPredicates, 1, MPI_INT, MPI_SUM, GlobalConstants::MASTER, MPI_COMM_WORLD);
	MPI_Reduce(&zero, &numberOfCorrectTriples, 1, MPI_INT, MPI_SUM, GlobalConstants::MASTER, MPI_COMM_WORLD);
	numberOfDuplicatedTriples = numberOfSentTriples - numberOfCorrectTriples;

	if (numberOfCorrectTriples == 0)
	{
		std::cout << "\nError: no correct triples were found in the graph (maybe a wrong separation symbol has been issued?)\n";
		std::cout << "Resulting triples distribution:\n";
		std::cout << "	Number of Nodes = " << numberOfNodes << "\n";
		std::cout << "	Number of Predicates = " << numberOfPredicates << "\n";
		std::cout << "	Number of Correct Triples = " << numberOfCorrectTriples << "\n";
		std::cout << "	Number of Malformed Triples = " << numberOfMalformedTriples << "\n";
		std::cout << "	Number of Empty Triples = " << numberOfEmptyTriples << "\n";
		std::cout << "	Number of Duplicated Triples = " << numberOfDuplicatedTriples << "\n";
		std::cout << "	Number of Filtered Triples = " << numberOfFilteredTriples << "\n";
		std::cout << "	Number of Triples = " << numberOfTriples << "\n";
		return false;
	}

	std::cout << "	Mapping Creation Complete\n";

	saveMapping(inputFileURI, numberOfprocesses, numberOfCorrectTriples, numberOfMalformedTriples, numberOfEmptyTriples, numberOfDuplicatedTriples, numberOfFilteredTriples, numberOfTriples, numberOfNodes, numberOfPredicates);
	return true;
}

bool MasterMapper::isToFilter(std::string subject, std::string predicate, std::string object)
{
	MappingConfigurationFileManager *mcfm = MappingConfigurationFileManager::getInstance();
	bool excludeOnPartialMatch = mcfm->isToExcludeOnPartialMatch();

	for(std::string node : *(mcfm->getUnwantedNodes()))
	{
		if (node == subject || node == object || (excludeOnPartialMatch && (subject.find(node) != subject.npos || object.find(node) != object.npos) ))
			return true;
	}

	for(std::string edge : *(mcfm->getUnwantedEdges()))
	{
		if (edge == predicate || (excludeOnPartialMatch && predicate.find(edge) != predicate.npos))
			return true;
	}

	return false;
}


void MasterMapper::saveMapping(std::string inputFileURI, int numberOfProcesses, int numberOfCorrectTriples, int numberOfMalformedTriples, int numberOfEmptyTriples, int numberOfDuplicatedTriples, int numberOfFilteredTriples, int numberOfTriples, int numberOfNodes, int numberOfPredicates)
{
	std::cout << "	Saving Mapping\n";

	std::ofstream outputFileStream((PreprocessingFolderStructureManager::getMappingFolderPath(false) + "MappingStatistics").c_str());
	outputFileStream << "Details about the Mapping\n\n";
	outputFileStream << "Number of Processes = " << numberOfProcesses << "\n\n";
	outputFileStream << "Number of Nodes = " << numberOfNodes << "\n";
	outputFileStream << "Number of Predicates = " << numberOfPredicates << "\n";
	outputFileStream << "Number of Correct Triples = " << numberOfCorrectTriples << "\n";
	outputFileStream << "Number of Malformed Triples = " << numberOfMalformedTriples << "\n";
	outputFileStream << "Number of Empty Triples = " << numberOfEmptyTriples << "\n";
	outputFileStream << "Number of Duplicated Triples = " << numberOfDuplicatedTriples << "\n";
	outputFileStream << "Number of Filtered Triples = " << numberOfFilteredTriples << "\n";
	outputFileStream << "Number of Triples = " << numberOfTriples << "\n";
	outputFileStream.close();

	outputFileStream.open((PreprocessingFolderStructureManager::getMappingFolderPath(true) + "Mapping.dat").c_str(), std::ios::binary);
	outputFileStream.write((char*)&numberOfProcesses, sizeof(int));
	outputFileStream.close();

	std::cout << "	Saving Complete\n";
}

void MasterMapper::loadMapping()
{
	std::cout << "	Loading Mapping\n";
	int operationType = LOAD_MAPPING;
	MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
	std::cout << "	Loading Complete\n";
}

void MasterMapper::terminateMapping()
{
	int operationType = TERMINATE_MAPPING;
	MPI_Bcast(&operationType, 1, MPI_INT, GlobalConstants::MASTER, MPI_COMM_WORLD);
}

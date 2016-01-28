
#include "PartitioningConfigurationFileManager.h"

PartitioningConfigurationFileManager * PartitioningConfigurationFileManager::instance = NULL;

PartitioningConfigurationFileManager::PartitioningConfigurationFileManager()
{
	alertFrequency = 0;
	maximumThreshold = 0;
	minimumThreshold = 0;
	expansionRatio = 0.0;
	expansionPercentage = 0.0;
	respectExpansionRatio = false;
	respectExpansionPercentage = false;
	respectMaximumThreshold = false;
}

PartitioningConfigurationFileManager* PartitioningConfigurationFileManager::getInstance()
{
	if (instance == NULL)
	{
		instance = new PartitioningConfigurationFileManager();
	}

	return instance;
}


bool PartitioningConfigurationFileManager::loadConfigurationFile(std::string configurationFileURI)
{
	std::ifstream inputFileStream(configurationFileURI.c_str());
	if (!inputFileStream.good())
	{
		std::cout << "\nAn error occurred while attempting to open the partitioning configuration file.\n\n";
		inputFileStream.close();
		return false;
	}

	std::string key, value, line, alertFrequencyString, maximumThresholdString, minimumThresholdString, expansionRatioString, expansionPercentageString, haltingConditionString;

	char* buffer = (char*)malloc(GlobalConstants::MAX_CHARS_PER_LINE * sizeof(char));

	while (!inputFileStream.eof())
	{
		inputFileStream.getline(buffer, GlobalConstants::MAX_CHARS_PER_LINE);

		if (buffer[0] == '\0')
		{
			continue;
		}

		line.assign(buffer);

		if (line.find_first_of(ConfigurationConstants::FIELD_SEPARATION_SYMBOL) == line.npos || line.find_first_of(ConfigurationConstants::COMMENT_STARTING_SYMBOL) == 0)
		{
			continue;
		}

		key = line.substr(0, line.find_first_of(ConfigurationConstants::FIELD_SEPARATION_SYMBOL));
		value = line.substr(line.find_first_of(ConfigurationConstants::FIELD_SEPARATION_SYMBOL) + 1, line.npos);

		if (key == "alert_frequency")
		{
			alertFrequencyString = value;
		}

		if (key == "maximum_threshold")
		{
			maximumThresholdString = value;
		}

		if (key == "minimum_threshold")
		{
			minimumThresholdString = value;
		}

		if (key == "expansion_ratio")
		{
			expansionRatioString = value;
		}

		if (key == "expansion_percentage")
		{
			expansionPercentageString = value;
		}

		if (key == "halting_condition")
		{
			haltingConditionString = value;
		}
	}

	inputFileStream.close();
	return checkParameters(alertFrequencyString, maximumThresholdString, minimumThresholdString, expansionRatioString, expansionPercentageString, haltingConditionString);
}

bool PartitioningConfigurationFileManager::checkParameters(std::string alertFrequencyString, std::string maximumThresholdString, std::string minimumThresholdString, std::string expansionRatioString, std::string expansionPercentageString, std::string haltingConditionString)
{
	if (alertFrequencyString.empty()) {
		std::cout << "\nError: the alert frequency is missing in the partitioning configuration file\n\n";
		return false;
	}

	if (maximumThresholdString.empty()) {
		std::cout << "\nError: the maximum threshold is missing in the partitioning configuration file\n\n";
		return false;
	}

	if (minimumThresholdString.empty()) {
		std::cout << "\nError: the minimum threshold is missing in the partitioning configuration file\n\n";
		return false;
	}

	if (expansionRatioString.empty()) {
		std::cout << "\nError: the expansion ratio is missing in the partitioning configuration file\n\n";
		return false;
	}

	if (expansionPercentageString.empty()) {
		std::cout << "\nError: the expansion percentage is missing in the partitioning configuration file\n\n";
		return false;
	}

	if (haltingConditionString.empty()) {
		std::cout << "\nError: the halting condition is missing in the partitioning configuration file\n\n";
		return false;
	}

	alertFrequency = atoi(alertFrequencyString.c_str());
	maximumThreshold = atoi(maximumThresholdString.c_str());
	minimumThreshold = atoi(minimumThresholdString.c_str());
	expansionPercentage = atof(expansionPercentageString.c_str());
	expansionRatio = atof(expansionRatioString.c_str());

	if (alertFrequency < 1)
	{
		std::cout << "\nError: the alert frequency must be an integer > 0 in the partitioning configuration file\n\n";
		return false;
	}

	if (maximumThreshold < 1)
	{
		std::cout << "\nError: the maximum threshold must be an integer > 0 in the partitioning configuration file\n\n";
		return false;
	}

	if (minimumThreshold < 1)
	{
		std::cout << "\nError: the minimum threshold must be an integer > 0 in the partitioning configuration file\n\n";
		return false;
	}

	if (minimumThreshold > maximumThreshold)
	{
		std::cout << "\nError: the minimum threshold is greater than the maximum threshold in the partitioning configuration file\n\n";
		return false;
	}

	if (expansionPercentage <= 0)
	{
		std::cout << "\nError: the expansion percentage must be a double > 0 in the partitioning configuration file\n\n";
		return false;
	}

	if (expansionPercentage > 1)
	{
		std::cout << "\nError: the expansion percentage must be a double <= 1 in the partitioning configuration file\n\n";
		return false;
	}

	if (expansionRatio <= 0)
	{
		std::cout << "\nError: the expansion ratio must be a double > 0 in the partitioning configuration file\n\n";
		return false;
	}

	if (expansionRatio > 1)
	{
		std::cout << "\nError: the expansion ratio must be a double <= 1 in the partitioning configuration file\n\n";
		return false;
	}

	std::string condition = "";

	if (haltingConditionString.substr(haltingConditionString.length() - ConfigurationConstants::VALUES_SEPARATION_SYMBOL.length(), ConfigurationConstants::VALUES_SEPARATION_SYMBOL.length()) != ConfigurationConstants::VALUES_SEPARATION_SYMBOL)
	{
		haltingConditionString += ConfigurationConstants::VALUES_SEPARATION_SYMBOL;
	}

	while (haltingConditionString.find(ConfigurationConstants::VALUES_SEPARATION_SYMBOL) != haltingConditionString.npos)
	{
		condition = haltingConditionString.substr(0, haltingConditionString.find(ConfigurationConstants::VALUES_SEPARATION_SYMBOL));

		if (condition == "maximum_threshold")
		{
			respectMaximumThreshold = true;
			haltingConditionString.erase(0, condition.length() + ConfigurationConstants::VALUES_SEPARATION_SYMBOL.length());
			continue;
		}

		if (condition == "expansion_percentage")
		{
			respectExpansionPercentage = true;
			haltingConditionString.erase(0, condition.length() + ConfigurationConstants::VALUES_SEPARATION_SYMBOL.length());
			continue;
		}

		if (condition == "expansion_ratio")
		{
			respectExpansionRatio = true;
			haltingConditionString.erase(0, condition.length() + ConfigurationConstants::VALUES_SEPARATION_SYMBOL.length());
			continue;
		}

		std::cout << "\nError: halting condition \"" << condition << "\" unknown in the partitioning configuration file\n\n";
		return false;
	}

	return true;
}


int PartitioningConfigurationFileManager::getAlertFrequency()
{
	return alertFrequency;
}

int PartitioningConfigurationFileManager::getMaximumThreshold()
{
	return maximumThreshold;
}

int PartitioningConfigurationFileManager::getMinimumThreshold()
{
	return minimumThreshold;
}

double PartitioningConfigurationFileManager::getexpansionRatio()
{
	return expansionRatio;
}

double PartitioningConfigurationFileManager::getexpansionPercentage()
{
	return expansionPercentage;
}

bool PartitioningConfigurationFileManager::isToRespectExpansionPercentage()
{
	return respectExpansionPercentage;
}

bool PartitioningConfigurationFileManager::isToRespectExpansionRatio()
{
	return respectExpansionRatio;
}

bool PartitioningConfigurationFileManager::isToRespectMaximumThreshold()
{
	return respectMaximumThreshold;
}

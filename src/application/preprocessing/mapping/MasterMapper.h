
#ifndef MASTERMAPPER_H_
#define MASTERMAPPER_H_

#include "Mapper.h"

class MasterMapper: public Mapper {

private:
	bool checkMappingExistence(int);
	bool createMapping(std::string, int);
	bool isToFilter(std::string, std::string, std::string);
	void saveMapping(std::string, int, int, int, int, int, int, int, int, int);
	void loadMapping();
	void terminateMapping();


public:
	bool executeMapping(int, std::string, std::string);
};

#endif

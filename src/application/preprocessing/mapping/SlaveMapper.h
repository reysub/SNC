#ifndef SLAVEMAPPER_H_
#define SLAVEMAPPER_H_

#include "Mapper.h"
#include <set>
#include <map>				// maps
#include <vector>			// vectors
#include <sstream>			// stringstream
#include <tr1/functional>	// hash function

class SlaveMapper: public Mapper {

private:
	bool hasToLoad;
	std::map<int, std::string> *mappingId2Node;
	std::map<int, std::string> *mappingId2Predicate;
	std::vector<int> *predicates;
	std::vector<int> *objects;
	std::map<int, std::vector<int> > *mappingNode2Positions;

	void checkMappingExistence(int);
	void createMapping(int, int);
	void saveMapping(int);
	void loadMapping(int);

public:
	SlaveMapper(std::map<int, std::string>*, std::map<int, std::string>*, std::vector<int>*, std::vector<int>*, std::map<int, std::vector<int> >*);
	void executeMapping(int, int, std::string);
};

#endif

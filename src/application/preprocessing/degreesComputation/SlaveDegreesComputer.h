
#ifndef SLAVEDEGREESCOMPUTER_H_
#define SLAVEDEGREESCOMPUTER_H_

#include "DegreesComputer.h"
#include <map>		// maps
#include <vector>	// vectors
#include <sstream>	// stringstream

class SlaveDegreesComputer: public DegreesComputer {

private:
	bool hasToLoad;
	std::map<int, std::string> *mappingId2Node;
	std::map<int, std::vector<int> > *mappingNode2Positions;
	std::map<int, int> *mappingNode2Cardinality;

	void checkDegreesComputationExistence(int, int);
	void createDegreesComputation(int, int, int);
	void saveDegreesComputation(int, int);
	void loadDegreesComputation(int, int);

public:
	SlaveDegreesComputer(std::map<int, std::string>*, std::map<int, std::vector<int> >*, std::map<int, int>*);
	void executeDegreesComputation(int, int, std::string, int);
};

#endif

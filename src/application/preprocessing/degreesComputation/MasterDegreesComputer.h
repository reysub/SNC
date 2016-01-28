
#ifndef MASTERDEGREESCOMPUTER_H_
#define MASTERDEGREESCOMPUTER_H_

#include "DegreesComputer.h"
#include <sstream>

class MasterDegreesComputer: public DegreesComputer {

private:
	bool checkDegreesComputationExistence(int);
	void createDegreesComputation(int, int);
	void saveDegreesComputation(int, int, int, int);
	void loadDegreesComputation();
	void terminateDegreesComputation();

public:
	bool executeDegreesComputation(int, std::string, int, int);

};

#endif

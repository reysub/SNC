#ifndef SLAVEFACADEPREPROCESSING_H_
#define SLAVEFACADEPREPROCESSING_H_

#include "FacadePreprocessing.h"
#include "../mapping/SlaveMapper.h"
#include "../degreesComputation/SlaveDegreesComputer.h"
#include "../partitioning/SlavePartitioner.h"

class SlaveFacadePreprocessing: public FacadePreprocessing {

private:
	std::map<int, std::string> mappingId2Node;
	std::map<int, std::string> mappingId2Predicate;
	std::vector<int> predicates, objects;
	std::map<int, std::vector<int> > mappingNode2Positions;
	std::map<int, int> mappingNode2Cardinality;

public:
	void executePreprocessing(int, int);
	std::string getPartitionUri(int, int);
};

#endif

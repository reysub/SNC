#ifndef MASTERFACADEPREPROCESSING_H_
#define MASTERFACADEPREPROCESSING_H_

#include "FacadePreprocessing.h"
#include "../mapping/MasterMapper.h"
#include "../degreesComputation/MasterDegreesComputer.h"
#include "../partitioning/MasterPartitioner.h"

class MasterFacadePreprocessing: public FacadePreprocessing {

public:
	bool executePreprocessing(int, std::vector<int>*);
};

#endif

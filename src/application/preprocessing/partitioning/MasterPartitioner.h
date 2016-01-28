#ifndef MASTERPARTITIONER_H_
#define MASTERPARTITIONER_H_

#include "Partitioner.h"
#include <sstream>		// stringstream

class MasterPartitioner: public Partitioner {

private:
	int createPartitioning(int, int);
	bool removeNonBlackListNodes();

	void initializeBlackListSet(int);

	void sendVector(std::vector<int>*);
	void receiveDataAndKernelCardinalities(int, std::map<int, int>*, std::set<int>*);
	void receiveGlobalCardinalities(int, std::set<int>*, std::map<int, int>*);
	void clearData(std::set<int>*, std::set<int>*);

	int getSuggestion(int, int, int*, std::set<int>*, std::vector<int>*);
	void selectNodesToExpand(std::set<int>*, std::vector<int>*, std::map<int, int>*, std::map<int, int>*, int, int, int, double, double, bool, bool, bool);
	std::vector<int> sortNodes(std::set<int>*, std::map<int, int>*, std::map<int, int>*);

	void addAllToVisited(std::vector<int>*);
	void retrieveTextualRepresentationAndTriples(int, std::vector<int>*, std::map<int, std::string>*, std::map<int, std::string>*, std::vector<int>*);

	void retrieveNodesTextualRepresentation(int, std::vector<int>*, std::map<int, std::string>*);
	void retrievePredicatesTextualRepresentation(int, std::vector<int>*, std::map<int, std::string>*);
	void retrieveTriples(int, std::vector<int>*, std::vector<int>*, std::vector<int>*);



	void savePartition(std::vector<int>*, std::set<int>*, std::set<int>*, std::vector<int>*, std::map<int, std::string>*, std::map<int, std::string>*, int, int, int, int);
	void saveDetails(int, int, int, int);

	void testPrint(std::vector<int>*, std::set<int>*, std::set<int>*, std::vector<int>*, std::set<int>*, std::set<int>*, std::set<int>*, std::set<int>*, std::map<int, int>*, std::map<int, int>*);

public:
	int executePartitioning(int, std::string, int);
	bool checkPartitioningExistence(std::vector<int>*, int);
};

#endif

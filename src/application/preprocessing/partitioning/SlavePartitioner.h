#ifndef SLAVEPARTITIONER_H_
#define SLAVEPARTITIONER_H_

#include "Partitioner.h"

class SlavePartitioner: public Partitioner {

private:
	std::map<int, std::string> *mappingId2Node;
	std::map<int, std::string> *mappingId2Predicate;
	std::vector<int> *predicates, *objects;
	std::map<int, std::vector<int> > *mappingNode2Positions;
	std::map<int, int> *mappingNode2Cardinality;
	std::set<int> blackListNodes;

	void createPartitioning(int, int);
	void initializeBlackListSet(int, int, int);
	void sendVector(std::vector<int>*);

	void removeNonBlackListNodes();

	void getSuggestion(int, int, std::set<int>*);
	void addAllToVisited(std::set<int>*);
	void retrieveTextualRepresentationAndTriples();
	void normalExecution(std::set<int>*);

public:
	SlavePartitioner(std::map<int, std::string>*, std::map<int, std::string>*, std::vector<int>*, std::vector<int>*, std::map<int, std::vector<int> >*, std::map<int, int>*);
	void executePartitioning(int, int, std::string);
};

#endif

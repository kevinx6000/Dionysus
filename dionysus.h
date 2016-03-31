#ifndef DIONYSUS_H
#define DIONYSUS_H

// Header
#include "header.h"
#include "structure.h"

// Dionysus simulator
class Dionysus{
	public:
		void readTopo(void);					// Read topology
		void readFlow(void);					// Read flow
		void genDepGraph(void);					// Generate dependency graph
		void requireCap(int, int, int, double);	// Require link capacity (add dependency)
		void releaseCap(int, int, int, double);	// Release link capacity (add dependency)
		void debug(void);						// For debugging
		int findDstPort(int, int);				// Find the port ID of destination switch
		int createNode(int, int);				// Create node
	private:
		vector<Switch>switches;					// Switch node
		vector<Link>links;						// Link node
		vector<Path>paths;						// Path node
		vector<Flow>allFlow;					// All flows
		vector<Node>nodes;						// List of node in dependency graph
		vector<int>mapID;						// Recording the mapping from 'actual ID' to 'current index'
};

#endif

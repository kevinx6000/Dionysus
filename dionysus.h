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
		bool calculateCPL(void);				// Calculate the CPL in dependency graph
		void updateGraph(void);					// Update current dependency graph
		void sortCPL(void);						// Sort nodes according to their CPL
		void debug(void);						// For debugging
	private:
		vector<Switch>switches;					// Switch node
		vector<Link>links;						// Link node
		vector<Path>paths;						// Path node
		vector<Operation>operations;			// Operation node
		vector<Flow>allFlow;					// All flows
		vector<Node>nodes;						// List of node in dependency graph
		vector<int>mapID;						// Record the mapping from 'actual ID' to 'current index'
		vector<int>addOpList;					// List of current Op ADD ID
		vector<int>delOpList;					// List of current Op DEL ID
		void requireCap(int, int, int, double);	// Require link capacity (add dependency)
		void releaseCap(int, int, int, double);	// Release link capacity (add dependency)
		void insertOpADD(int, int, double=0);	// Insert OP_ADD node into dependency graph
		void insertOpDEL(int);					// Insert OP_DEL node into dependency graph
		static bool cmpCPL(Node, Node);			// Comparison function for sorting CPL
		int findDstPort(int, int);				// Find the port ID of destination switch
		int createNode(int, int);				// Create node
		int dfsCPL(int);						// DFS for CPL
};

#endif

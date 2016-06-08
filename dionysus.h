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
		void start(void);						// Start scheduling
		void debug(void);						// For debugging
		void statistic(void);					// Output statistic report
	private:
		vector<Switch>switches;					// Switch node
		vector<Link>links;						// Link node
		vector<TrancNode>trancNode;				// Transceiver node: Wireless AP bandwidth guarantee
		vector<InterNode>interNode;				// Interference node: Interference processing
		vector<Path>paths;						// Path node
		vector<Operation>operations;			// Operation node
		vector<Flow>allFlow;					// All flows
		vector<Node>nodes;						// List of node in dependency graph
		vector<int>mapID;						// Record the mapping from 'actual ID' to 'current index'
		int addRuleCnt;							// Total number of add rule
		int modRuleCnt;							// Total number of modify rule
		int delRuleCnt;							// Total number of delete rule
		int roundCnt;							// Total number of round
		void requireCap(int, int, int, double);	// Require link capacity (add dependency)
		void releaseCap(int, int, int, double);	// Release link capacity (add dependency)
		void updateGraph(void);					// Update current dependency graph
		void sortCPL(void);						// Sort nodes according to their CPL
		void schedule(int);						// Schedule operation Oi
		bool calculateCPL(void);				// Calculate the CPL in dependency graph
		bool canSchedule(int);					// Check if some operation o can be scheduled now
		static bool cmpCPL(Node, Node);			// Comparison function for sorting CPL
		static bool cmpPATH(Link, Link);		// Comparison function for sorting path links
		int findDstPort(int, int);				// Find the port ID of destination switch
		int createNode(int, int);				// Create node
		int dfsCPL(int);						// DFS for CPL
		double vecdot(double[], double[], double[], double[]);	// Vector dot
		double vecdis(double[], double[], double[], double[]);	// Vector distance
};

#endif

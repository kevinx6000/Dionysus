#ifndef STRUCTURE_H
#define STRUCTURE_H

// Header
#include "header.h"

// Switch node
class Switch{
	public:
		int dpID;			// ID for dependency graph
		int switchID;		// ID of switch
		int tcamUsage;		// Current TCAM usage
		vector<int>port;	// The switch ID this port connects to
		vector<int>linkID;	// The link ID of this port
};

// Link node
class Link{
	public:
		int dpID;			// ID for dependency graph
		int sourceID;		// Source switch ID
		int destinationID;	// Destination switch ID
		double linkCapacity;// Link capacity
};

// Flow path
class FlowPath{
	public:
		double traffic;
		vector<Link>link[2];
};

// Flow
class Flow{
	public:
		int ingressID;
		vector<FlowPath>flowPath;
};

// Edge (for dependency graph)
class Edge{
	public:
		int nodeID;
		int intWeight;
		double dobWeight;
};

// Node (for dependency graph)
enum{ RES_SWITCH, RES_LINK, PATH, OPERATION };
class Node{
	public:
		int nodeID;			// Node in dependency graph
		int nodeType;		// Node type (Switch/Link/Path/Operation)
		int nodeIndex;		// Node index of that kind of node
		int CPL;			// Record the CPL value
		vector<Edge>child;	// List of children node
		vector<int>parent;	// List of parent node (ID only)
};

// Path node
class Path{
	public:
		int dpID;			// ID for dependency graph
		double available;	// Available traffic for movement
		double committed;	// Moved traffic on this path
};

// Operation node
enum{ OP_ADD, OP_MOD, OP_DEL };
class Operation{
	public:
		int dpID;			// ID for dependency graph
		int operationType;	// Type of operation(add/mod/del)
		int switchID;		// ID of switch this operation works on
		bool isFinished;	// Mark the finish state of that operation
};

#endif

#ifndef STRUCTURE_H
#define STRUCTURE_H

// Header
#include "header.h"

// Switch node
typedef struct{
	int dpID;			// ID for dependency graph
	int switchID;		// ID of switch
	int tcamUsage;		// Current TCAM usage
	vector<int>port;	// The switch ID this port connects to
	vector<int>linkID;	// The link ID of this port
}Switch;

// Link node
typedef struct{
	int dpID;			// ID for dependency graph
	int sourceID;		// Source switch ID
	int destinationID;	// Destination switch ID
	double linkCapacity;// Link capacity
}Link;

// Flow Path
typedef struct{
	double traffic;
	vector<Link>link[2];
}FlowPath;

// Flow
typedef struct{
	int ingressID;
	vector<FlowPath>flowPath;
}Flow;

/*
// Path node
typedef struct{
	int dpID;			// ID for dependency graph
	double available;	// Available traffic for movement
	double committed;	// Moved traffic on this path
}Path;

// Operation node
enum{ OP_ADD, OP_MOD, OP_DEL };
typedef struct{
	int dpID;			// ID for dependency graph
	int operationType;	// Type of operation(add/mod/del)
	int switchID;		// ID of switch this operation works on
	bool isFinished;	// Mark the finish state of that operation
}Operation;

// Switch INFO (for traffic distribution)
typedef struct{
	int switchID;			// Switch ID
	vector<double>volume;	// Traffic volume for each port
}SwitchINFO;

// Flow
typedef struct{
	int ingressID;					// Ingress switch ID
	vector<SwitchINFO>traffic[2];	// Initial/Final traffic distribution
}Flow;

// Edge (for dependency graph)
typedef struct{
	int nodeID;
	int intWeight;
	double dobWeight;
}Edge;

// Node (for dependency graph)
enum{ RES_SWITCH, RES_LINK, PATH, OPERATION };
typedef struct{
	int nodeID;			// Node in dependency graph
	int nodeType;		// Node type (Switch/Link/Path/Operation)
	int nodeIndex;		// Node index of that kind of node
	int CPL;			// Record the CPL value
	vector<Edge>child;	// List of children node
	vector<int>parent;	// List of parent node (ID only)
}Node;

// Dependency graph
typedef struct{
	vector<Node>nodes;	// List of node in dependency graph
	vector<int>mapID;	// Recording the mapping from 'actual ID' to 'current index'
}DPGraph;
*/

#endif

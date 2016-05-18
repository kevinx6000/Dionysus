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
		double posXY[2];	// X/Y position of ToR switch
		int trancID;		// Transceiver node ID
		int interID;		// Interference node ID
};

// Link node
class Link{
	public:
		int dpID;			// ID for dependency graph
		int sourceID;		// Source switch ID
		int destinationID;	// Destination switch ID
		double linkCapacity;// Link capacity
		vector<double>curTraffic;	// Current traffic of this link of some flow
		bool isWireless;	// Mark as wireless or not
		vector<int>iList;	// Interference list
};

// Transceiver Node
class TrancNode{
	public:
		int dpID;			// ID for dependency graph
		int switchID;		// ID of switch
		int releaseCnt;		// Number of release on this node
		int requireCnt;		// Number of require on this node
		double nodeCapacity;// Node capacity
};

// Interference Node
class InterNode{
	public:
		int dpID;			// ID for dependency graph
		int switchID;		// ID of switch
		int releaseCnt;		// Number of release on this node
		int requireCnt;		// Number of require on this node
		double nodeCapacity;// Node capacity
};

// Flow path
class FlowPath{
	public:
		int dstID[2];
		double traffic;
		vector<Link>link[2];
};

// Flow
class Flow{
	public:
		int flowTag;		// Record flow name
		int flowID;			// Record flow ID in structure
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
enum{ RES_SWITCH, RES_LINK, RES_TRANC, RES_INTER, PATH, OPERATION };
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
		int pairID;			// ID for pair of path node (pIn<>pOut)
		int flowPathID;		// ID for corresponding flowPath of some flow
};

// Rule set
enum{ RULE_ADD, RULE_MOD, RULE_DEL };
class Rule{
	public:
		int switchID;			// Switch ID of this rule
		int ruleType;			// Rule type (add/mod/del)
		vector<double>traffic;	// Traffic for each port on this switch
};

// Operation node
enum{ OP_ADD, OP_MOD, OP_DEL };
class Operation{
	public:
		int dpID;			// ID for dependency graph
		int operationType;	// Type of operation(add/mod/del)
		int switchID;		// ID of switch this operation works on
		int flowID;			// ID of flow this operation works on
		int flowTag;		// Name of flow this operation works on
		bool isFinished;	// Mark the finish state of that operation
		vector<Rule>ruleSet;// Rule set
};

#endif

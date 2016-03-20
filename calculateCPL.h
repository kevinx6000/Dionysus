#ifndef CALCULATE_CPL_H
#define CALCULATE_CPL_H

#include "header.h"
#include "structure.h"

// DFS for CPL
int dfsCPL(DPGraph &dpGraph, int nowID){

	// CYCLE FOUND!!!
	if(dpGraph.CPL[nowID] == -2) return -1;

	// Already done
	if(dpGraph.CPL[nowID] != -1) return dpGraph.CPL[nowID];

	// Set CPL as -2 for cycle detection
	dpGraph.CPL[nowID] = -2;

	// Recursive
	int maxCPL = 0;
	for(int i = 0; i < dpGraph.nodes[nowID].child.size(); i++){
		int nxtID = dpGraph.nodes[nowID].child[i].nodeID;
		int tmp = dfsCPL(dpGraph, nxtID);
		if(tmp == -1) return -1;
		if(tmp > maxCPL) maxCPL = tmp;
	}

	// Record CPL
	if(dpGraph.nodes[nowID].nodeType == OPERATION) maxCPL++;
	dpGraph.CPL[nowID] = maxCPL;
	return maxCPL;
}

// Calculate CPL
bool calculateCPL(DPGraph &dpGraph){

	// Clear CPL
	for(int i = 0; i < dpGraph.CPL.size(); i++)
		dpGraph.CPL[i] = -1;

	// Start from indegree = 0
	for(int i = 0; i < dpGraph.nodes.size(); i++){
		if(!dpGraph.nodes[i].parent.size()){
			int tmp = dfsCPL(dpGraph, dpGraph.nodes[i].nodeID);
			if(tmp == -1) return false;
		}
	}
	return true;
}

#endif

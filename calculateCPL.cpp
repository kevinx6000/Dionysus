#ifndef CALCULATE_CPL_H
#define CALCULATE_CPL_H

// Header
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// DFS for CPL
int Dionysus::dfsCPL(int nowID){

	// CYCLE FOUND!!!
	if(nodes[nowID].CPL == -2) return -1;

	// Already done
	if(nodes[nowID].CPL != -1) return nodes[nowID].CPL;

	// Set CPL as -2 for cycle detection
	nodes[nowID].CPL = -2;
	
	// Recursive
	int maxCPL = 0;
	for(int i = 0; i < (int)nodes[nowID].child.size(); i++){
		int nxtID = mapID[ nodes[nowID].child[i].nodeID ];
		int tmp = dfsCPL(nxtID);
		if(tmp == -1) return -1;
		if(tmp > maxCPL) maxCPL = tmp;
	}

	// Record CPL
	if(nodes[nowID].nodeType == OPERATION) maxCPL++;
	nodes[nowID].CPL = maxCPL;
	return maxCPL;
}

// Calculate CPL
bool Dionysus::calculateCPL(void){

	// Clear CPL
	for(int i = 0; i < (int)nodes.size(); i++)
		nodes[i].CPL = -1;

	// Start from indegree = 0
	for(int i = 0; i < (int)nodes.size(); i++){
		if((int)nodes[i].parent.size() == 0){
			int tmp = dfsCPL(mapID[ nodes[i].nodeID ]);
			if(tmp == -1) return false;
		}
	}
	return true;
}
#endif

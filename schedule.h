#ifndef SCHEDULE_H
#define SCHEDULE_H

// Header
#include "header.h"
#include "structure.h"

// Schedule operation: add/mod/del rule
void schedule(DPGraph &dpGraph, vector<Path> &paths, vector<Operation> &operations, int owID){

	// Variables
	int owDpID, oaDpID, oaID, odDpID, odID, pDpID, pID, rDpID, rID;
	bool addRule, isAdded;

	// Initialize
	owDpID = dpGraph.mapID[ operations[owID].dpID ];

	// Add rule with new version
	isAdded = false;
	for(int i = 0; i < dpGraph.nodes[owDpID].parent.size(); i++){

		// For all parent nodes (actually, there's only one path node as parent)
		addRule = false;
		oaDpID = dpGraph.mapID[ dpGraph.nodes[owDpID].parent[i] ];
		for(int j = 0; j < dpGraph.nodes[oaDpID].parent.size(); j++){
			pDpID = dpGraph.mapID[ dpGraph.nodes[oaDpID].parent[j] ];
			if(dpGraph.nodes[pDpID].nodeType == PATH){

				// Traffic available for this ADD operation
				pID = dpGraph.nodes[pDpID].nodeIndex;
				if(paths[pID].available > 0.0){
					addRule = isAdded = true;
					break;
				}
			}
			else if(dpGraph.nodes[pDpID].nodeType == RES_SWITCH)
				rDpID = pDpID;
		}

		// Add rule with new version if traffic available > 0
		if(addRule){
			oaID = dpGraph.nodes[oaDpID].nodeIndex;
			printf("Add/Modify rule @ switch %d\n", operations[oaID].switchID);
		}
	}

	// Modify rule @ ingress switch with new version
	if(isAdded){
		printf("Modify rule @ ingress switch %d\n", operations[owID].switchID);
	}

	// Delete rule with old version
	if(isAdded){
		for(int i = 0; i < dpGraph.nodes[owDpID].child.size(); i++){
			odDpID = dpGraph.mapID[ dpGraph.nodes[owDpID].child[i].nodeID ];
			if(dpGraph.nodes[odDpID].nodeType == OPERATION){
				odID = dpGraph.nodes[odDpID].nodeIndex;
				printf("Delete rule @ switch %d\n", operations[odID].switchID);
			}
		}
	}
}

#endif

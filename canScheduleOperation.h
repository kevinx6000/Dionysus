#ifndef CAN_SCHEDULE_OPERATION_H
#define CAN_SCHEDULE_OPERATION_H

// Headers
#include "header.h"
#include "structure.h"

// Check whether an operation is able to be scheduled
bool canScheduleOperation(DPGraph &dpGraph, vector<Switch> &switches, vector<Link> &links, 
		vector<Path> &paths, vector<Operation> &operations, int owDpID){

	// Variables
	int owID, oaDpID, pDpID, pID, rDpID, rID;
	bool canSchedule;
	double total, avTmp;
	Operation otmp;

	// We only check for weight change operations
	owID = dpGraph.nodes[owDpID].nodeIndex;
	if(dpGraph.nodes[owDpID].nodeType != OPERATION
			|| operations[owID].operationType != OP_MOD) return false;
	canSchedule = false;

	// Check link capacity resource
	total = 0.0;
	oaDpID = dpGraph.mapID[ dpGraph.nodes[ dpGraph.mapID[ dpGraph.nodes[owDpID].parent[0] ] ].nodeID ];
	for(int i = 0; i < dpGraph.nodes[oaDpID].parent.size(); i++){

		// Path nodes
		pDpID = dpGraph.mapID[ dpGraph.nodes[oaDpID].parent[i] ];
		if(dpGraph.nodes[pDpID].nodeType == PATH){

			// Resource nodes
			avTmp = 1e9;
			for(int j = 0; j < dpGraph.nodes[pDpID].parent.size(); j++){

				// Link
				rDpID = dpGraph.mapID[ dpGraph.nodes[pDpID].parent[j] ];
				if(dpGraph.nodes[rDpID].nodeType == RES_LINK){
					
					// Remaining resource
					rID = dpGraph.nodes[rDpID].nodeIndex;
					avTmp = min(avTmp, links[rID].linkCapacity);

					// Link -> Path (only one link fits)
					for(int k = 0; k < dpGraph.nodes[rDpID].child.size(); k++)
						if(dpGraph.mapID[ dpGraph.nodes[rDpID].child[k].nodeID ] == pDpID)
							avTmp = min(avTmp, dpGraph.nodes[rDpID].child[k].dobWeight);
				}
			}
			if(avTmp < 1e9) total += avTmp;
			else avTmp = 0.0;
			paths[ dpGraph.nodes[pDpID].nodeIndex ].available = avTmp;
		}
	}

	// Some flow can be move right now
	if(total > 0.0) canSchedule = true;

	// Check switch memeory resource
	for(int i = 0; i < dpGraph.nodes[owDpID].parent.size(); i++){

		// Operation nodes
		oaDpID = dpGraph.mapID[ dpGraph.nodes[owDpID].parent[i] ];
		if(dpGraph.nodes[oaDpID].nodeType == OPERATION){

			// Resource nodes
			for(int j = 0; j < dpGraph.nodes[oaDpID].parent.size(); j++){

				// Switch
				rDpID = dpGraph.mapID[ dpGraph.nodes[oaDpID].parent[j] ];
				if(dpGraph.nodes[rDpID].nodeType == RES_SWITCH){

					// Not enought TCAM
					rID = dpGraph.nodes[rDpID].nodeIndex;
					for(int k = 0; k < dpGraph.nodes[rDpID].child.size(); k++)
						if(dpGraph.mapID[ dpGraph.nodes[rDpID].child[k].nodeID ] == oaDpID &&
							switches[rID].tcamUsage < dpGraph.nodes[rDpID].child[k].intWeight)
							canSchedule = false;
				}
			}
		}
	}

	// Both links & switches are enough
	if(canSchedule){

		// Update link capacity resource
		oaDpID = dpGraph.mapID[ dpGraph.nodes[ dpGraph.mapID[ dpGraph.nodes[owDpID].parent[0] ] ].nodeID ];
		for(int i = 0; i < dpGraph.nodes[oaDpID].parent.size(); i++){

			// Path nodes
			pDpID = dpGraph.mapID[ dpGraph.nodes[oaDpID].parent[i] ];
			if(dpGraph.nodes[pDpID].nodeType == PATH){

				// Resource nodes
				avTmp = paths[ dpGraph.nodes[pDpID].nodeIndex ].available;
				for(int j = 0; j < dpGraph.nodes[pDpID].parent.size(); j++){

					// Link resource
					rDpID = dpGraph.mapID[ dpGraph.nodes[pDpID].parent[j] ];
					if(dpGraph.nodes[rDpID].nodeType == RES_LINK){

						// Link capacity
						rID = dpGraph.nodes[rDpID].nodeIndex;
						links[rID].linkCapacity -= avTmp;

						// Link -> Path (only one link fits)
						for(int k = 0; k < dpGraph.nodes[rDpID].child.size(); k++)
							if(dpGraph.mapID[ dpGraph.nodes[rDpID].child[k].nodeID ] == pDpID)
								dpGraph.nodes[rDpID].child[k].dobWeight -= avTmp;
					}
				}

				/* NOTE: We skip line23-24 in algorithm6 since we don't have path node weight links */
			}
		}

		// Update switch memory resource
		for(int i = 0; i < dpGraph.nodes[owDpID].parent.size(); i++){

			// Operation nodes
			oaDpID = dpGraph.mapID[ dpGraph.nodes[owDpID].parent[i] ];
			if(dpGraph.nodes[oaDpID].nodeType == OPERATION){

				// Resource nodes
				for(int j = 0; j < dpGraph.nodes[oaDpID].parent.size(); j++){

					// Switch
					rDpID = dpGraph.mapID[ dpGraph.nodes[oaDpID].parent[j] ];
					if(dpGraph.nodes[rDpID].nodeType == RES_SWITCH){

						// Not enought TCAM
						rID = dpGraph.nodes[rDpID].nodeIndex;
						for(int k = 0; k < dpGraph.nodes[rDpID].child.size(); k++)
							if(dpGraph.mapID[ dpGraph.nodes[rDpID].child[k].nodeID ] == oaDpID)
								switches[rID].tcamUsage -= dpGraph.nodes[rDpID].child[k].intWeight;
					}
				}
			}
		}

		// Update the children of change weight operation
		for(int i = 0; i < dpGraph.nodes[owDpID].child.size(); i++){

			// Path nodes
			pDpID = dpGraph.mapID[ dpGraph.nodes[owDpID].child[i].nodeID ];
			if(dpGraph.nodes[pDpID].nodeType == PATH){

				// Committed traffic
				pID = dpGraph.nodes[pDpID].nodeIndex;
				paths[pID].committed = min(dpGraph.nodes[owDpID].child[i].dobWeight, total);
				dpGraph.nodes[owDpID].child[i].dobWeight -= paths[pID].committed;
				total -= paths[pID].committed;
			}
		}
	}
	
	// Return can be scheduling or not
	return canSchedule;
}

#endif

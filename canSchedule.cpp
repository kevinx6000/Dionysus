// Headers
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Check whether an operation is able to be scheduled
bool Dionysus::canSchedule(int owDpID){

	// Variables
	int owID, oaDpID, pDpID, pID, rDpID, rID;
	bool canSchedule;
	double total, avTmp;

	// We only check for weight change operations
	owID = nodes[owDpID].nodeIndex;
	if(nodes[owDpID].nodeType != OPERATION || 
			operations[owID].operationType != OP_MOD) return false;
	canSchedule = false;

	// Check link capacity resource
	total = 0.0;
	/* TODO: What if NO OP ADD as parent...? i.e., parent.size() = 0 */
	oaDpID = mapID[ nodes[owDpID].parent[0] ];
	for(int i = 0; i < (int)nodes[oaDpID].parent.size(); i++){

		// Path nodes
		pDpID = mapID[ nodes[oaDpID].parent[i] ];
		if(nodes[pDpID].nodeType == PATH){

			// Search for the operation child itself
			for(int j = 0; j < (int)nodes[pDpID].child.size(); j++)
				if(mapID[ nodes[pDpID].child[j].nodeID ] == oaDpID)
					avTmp = nodes[pDpID].child[j].dobWeight;

			// Resource nodes
			for(int j = 0; j < (int)nodes[pDpID].parent.size(); j++){

				// Link
				rDpID = mapID[ nodes[pDpID].parent[j] ];
				if(nodes[rDpID].nodeType == RES_LINK){
					
					// Remaining resource
					rID = nodes[rDpID].nodeIndex;
					avTmp = min(avTmp, links[rID].linkCapacity);

					// Link -> Path (only one link fits)
					for(int k = 0; k < (int)nodes[rDpID].child.size(); k++)
						if(mapID[ nodes[rDpID].child[k].nodeID ] == pDpID)
							avTmp = min(avTmp, nodes[rDpID].child[k].dobWeight);
				}
			}
			total += avTmp;
			paths[ nodes[pDpID].nodeIndex ].available = avTmp;
		}
	}

	// Some flow can be move right now
	if(total > 0.0) canSchedule = true;

	// Check switch memeory resource
	for(int i = 0; i < (int)nodes[owDpID].parent.size(); i++){

		// Operation nodes
		oaDpID = mapID[ nodes[owDpID].parent[i] ];
		if(nodes[oaDpID].nodeType == OPERATION){

			// Resource nodes
			for(int j = 0; j < (int)nodes[oaDpID].parent.size(); j++){

				// Switch
				rDpID = mapID[ nodes[oaDpID].parent[j] ];
				if(nodes[rDpID].nodeType == RES_SWITCH){

					// Not enought TCAM
					rID = nodes[rDpID].nodeIndex;
					for(int k = 0; k < (int)nodes[rDpID].child.size(); k++)
						if(mapID[ nodes[rDpID].child[k].nodeID ] == oaDpID &&
							switches[rID].tcamUsage < nodes[rDpID].child[k].intWeight)
							canSchedule = false;
				}
			}
		}
	}

	// Both links & switches are enough
	if(canSchedule){

		// Update link capacity resource
		oaDpID = mapID[ nodes[owDpID].parent[0] ];
		for(int i = 0; i < (int)nodes[oaDpID].parent.size(); i++){

			// Path nodes
			pDpID = mapID[ nodes[oaDpID].parent[i] ];
			if(nodes[pDpID].nodeType == PATH){

				// Resource nodes
				avTmp = paths[ nodes[pDpID].nodeIndex ].available;
				for(int j = 0; j < (int)nodes[pDpID].parent.size(); j++){

					// Link resource
					rDpID = mapID[ nodes[pDpID].parent[j] ];
					if(nodes[rDpID].nodeType == RES_LINK){

						// Link capacity
						rID = nodes[rDpID].nodeIndex;
						links[rID].linkCapacity -= avTmp;

						// Link -> Path (only one link fits)
						for(int k = 0; k < (int)nodes[rDpID].child.size(); k++)
							if(mapID[ nodes[rDpID].child[k].nodeID ] == pDpID)
								nodes[rDpID].child[k].dobWeight -= avTmp;
					}
				}

				// Path -> Operation
				for(int j = 0; j < (int)nodes[pDpID].child.size(); j++)
					if(mapID[ nodes[pDpID].child[j].nodeID ] == oaDpID)
						nodes[pDpID].child[j].dobWeight -= avTmp;
			}
		}

		// Update switch memory resource
		for(int i = 0; i < (int)nodes[owDpID].parent.size(); i++){

			// Operation nodes
			oaDpID = mapID[ nodes[owDpID].parent[i] ];
			if(nodes[oaDpID].nodeType == OPERATION){

				// Resource nodes
				for(int j = 0; j < (int)nodes[oaDpID].parent.size(); j++){

					// Switch
					rDpID = mapID[ nodes[oaDpID].parent[j] ];
					if(nodes[rDpID].nodeType == RES_SWITCH){

						// Update the switch node
						rID = nodes[rDpID].nodeIndex;
						for(int k = 0; k < (int)nodes[rDpID].child.size(); k++)
							if(mapID[ nodes[rDpID].child[k].nodeID ] == oaDpID){
								switches[rID].tcamUsage -= nodes[rDpID].child[k].intWeight;
//								nodes[rDpID].child[k].intWeight = 0;
							}
					}
				}
			}
		}

		// Update the children of change weight operation
		for(int i = 0; i < (int)nodes[owDpID].child.size(); i++){

			// Path nodes
			pDpID = mapID[ nodes[owDpID].child[i].nodeID ];
			if(nodes[pDpID].nodeType == PATH){

				// Committed traffic
				pID = nodes[pDpID].nodeIndex;
				paths[pID].committed = min(nodes[owDpID].child[i].dobWeight, total);
				nodes[owDpID].child[i].dobWeight -= paths[pID].committed;
				total -= paths[pID].committed;
			}
		}
	}
	
	// Return can be scheduling or not
	return canSchedule;
}

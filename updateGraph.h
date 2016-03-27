#ifndef UPDATE_GRAPH_H
#define UPDATE_GRAPH_H

// Headers
#include "header.h"
#include "structure.h"

// Update dependency graph
void updateGraph(DPGraph &dpGraph, vector<Switch> &switches, vector<Link> &links, 
		vector<Path> &paths, vector<Operation> &operations){

	// Variables
	int rDpID, pDpID, pID, oaDpID, oaID;

	// For each finished operation node
	for(int i = 0; i < dpGraph.nodes.size(); i++){
		if(dpGraph.nodes[i].nodeType == OPERATION
				&& operations[ dpGraph.nodes[i].nodeIndex ].isFinished){

			// Delete operation
			if(operations[ dpGraph.nodes[i].nodeIndex ].operationType == OP_DEL ){

				// Update resource (only one child: switch)
				rDpID = dpGraph.mapID[ dpGraph.nodes[i].child[0].nodeID ];
				switches[ dpGraph.nodes[rDpID].nodeIndex ].tcamUsage += dpGraph.nodes[i].child[0].intWeight;
			}

			// Change weight operation
			else if(operations[ dpGraph.nodes[i].nodeIndex ].operationType == OP_MOD){

				// Path nodes
				for(int j = 0; j < dpGraph.nodes[i].child.size(); j++){
					pDpID = dpGraph.mapID[ dpGraph.nodes[i].child[j].nodeID ];
					pID = dpGraph.nodes[pDpID].nodeIndex;
					if(dpGraph.nodes[pDpID].nodeType == PATH){

						// Resource nodes (links)
						for(int k = 0; k < dpGraph.nodes[pDpID].child.size(); k++){
							rDpID = dpGraph.mapID[ dpGraph.nodes[pDpID].child[k].nodeID ];
							links[ dpGraph.nodes[rDpID].nodeIndex ].linkCapacity += paths[pID].committed;
							dpGraph.nodes[pDpID].child[k].dobWeight -= paths[pID].committed;

							// Finish: all resource released on this link
							if(dpGraph.nodes[pDpID].child[k].dobWeight <= 0){
								dpGraph.nodes[pDpID].child.erase( dpGraph.nodes[pDpID].child.begin() + k );
								k--;
							}
						}
						paths[pID].committed = 0.0;

						// Finish: all resource released by operation
						if(dpGraph.nodes[i].child[j].dobWeight <= 0){
							dpGraph.nodes[pDpID].child.clear();
							dpGraph.nodes[pDpID].parent.clear();
							dpGraph.nodes[i].child.erase( dpGraph.nodes[i].child.begin() + j);
							j--;
						}
					}
				}

				// Finish: all paths and operations done
				if(dpGraph.nodes[i].child.size() == 0){

					// Delete all its parents
					for(int j = 0; j < dpGraph.nodes[i].parent.size(); j++){

						// And edges from parent to this node
						oaDpID = dpGraph.mapID[ dpGraph.nodes[i].parent[j] ];
						for(int k = 0; k < dpGraph.nodes[oaDpID].child.size(); k++){
							if(dpGraph.nodes[oaDpID].child[k].nodeID == dpGraph.nodes[i].nodeID){
								dpGraph.nodes[oaDpID].child.erase( dpGraph.nodes[oaDpID].child.begin() + k);
								break;
							}
						}
						dpGraph.nodes[i].parent.erase( dpGraph.nodes[i].parent.begin() + j );
						j--;
					}

					// TODO: Maybe we should mark finished here?
				}
			}
		}
	}
	
	// For each resource node
	for(int i = 0; i < dpGraph.nodes.size(); i++){

		// Switch
		if(dpGraph.nodes[i].nodeType == RES_SWITCH){

			// All needed resource
			int need = 0;
			for(int j = 0; j < dpGraph.nodes[i].child.size(); j++)
				need += dpGraph.nodes[i].child[j].intWeight;

			// TCAM usage is enough for all links
			int index = dpGraph.nodes[i].nodeIndex;
			if(switches[index].tcamUsage >= need){
//				switches[index].tcamUsage -= need;

				// Delete all out links
//				for(int j = 0; j < dpGraph.nodes[i].child.size(); j++){
//					int child = dpGraph.mapID[ dpGraph.nodes[i].child[j].nodeID ];
//					for(int k = 0; k < dpGraph.nodes[child].parent.size(); k++){
//						if(dpGraph.nodes[child].parent[k] == dpGraph.nodes[i].nodeID){
//							dpGraph.nodes[child].parent.erase(dpGraph.nodes[child].parent.begin()+k);
//							break;
//						}
//					}
//				}
				dpGraph.nodes[i].child.clear();
			}
		}

		// Links
		if(dpGraph.nodes[i].nodeType == RES_LINK){

			// All needed resource
			double need = 0;
			for(int j = 0; j < dpGraph.nodes[i].child.size(); j++)
				need += dpGraph.nodes[i].child[j].dobWeight;

			// Link usage is enough for all links
			int index = dpGraph.nodes[i].nodeIndex;
			if(links[index].linkCapacity >= need){
//				links[index].linkCapacity -= need;

				// Delete all out links
//				for(int j = 0; j < dpGraph.nodes[i].child.size(); j++){
//					int child = dpGraph.mapID[ dpGraph.nodes[i].child[j].nodeID ];
//					for(int k = 0; k < dpGraph.nodes[child].parent.size(); k++){
//						if(dpGraph.nodes[child].parent[k] == dpGraph.nodes[i].nodeID){
//							dpGraph.nodes[child].parent.erase(dpGraph.nodes[child].parent.begin()+k);
//							break;
//						}
//					}
//				}
				dpGraph.nodes[i].child.clear();
			}
		}
	}
}

#endif

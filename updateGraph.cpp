// Headers
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Update dependency graph
void Dionysus::updateGraph(void){

	// Variables
	int rDpID, pDpID, pID, oaDpID, odID;
	bool hasPathChild;

	// For each finished operation node
	for(int i = 0; i < (int)nodes.size(); i++){
		if(nodes[i].nodeType == OPERATION && operations[ nodes[i].nodeIndex ].isFinished){

			// Delete operation
			if(operations[ nodes[i].nodeIndex ].operationType == OP_DEL){

				// Update all switches' memory
				odID = nodes[i].nodeIndex;
				for(int j = 0; j < (int)operations[odID].ruleSet.size(); j++)
					switches[operations[odID].ruleSet[j].switchID].tcamUsage ++;

				// Mark back into un-finished
				operations[odID].isFinished = false;
			}

			// Change weight operation
			else if(operations[ nodes[i].nodeIndex ].operationType == OP_MOD){

				// Path nodes
				hasPathChild = false;
				for(int j = 0; j < (int)nodes[i].child.size(); j++){
					pDpID = mapID[ nodes[i].child[j].nodeID ];
					if(nodes[pDpID].nodeType == PATH){

						// Resource nodes (links)
						pID = nodes[pDpID].nodeIndex;
						for(int k = 0; k < (int)nodes[pDpID].child.size(); k++){
							rDpID = mapID[ nodes[pDpID].child[k].nodeID ];
							links[ nodes[rDpID].nodeIndex ].linkCapacity += paths[pID].committed;
							nodes[pDpID].child[k].dobWeight -= paths[pID].committed;

							// Finish: all resource released on this link
							if(nodes[pDpID].child[k].dobWeight <= 0){
								nodes[pDpID].child.erase( nodes[pDpID].child.begin() + k );
								k--;
							}
						}
						paths[pID].committed = 0.0;

						// Finish: all resource released by operation
						if((int)nodes[pDpID].child.size() == 0){
							nodes[pDpID].parent.clear();
							nodes[i].child.erase( nodes[i].child.begin() + j);
							j--;
						}
						else hasPathChild = true;
					}
				}

				// Finish: all paths and operations done
				if(!hasPathChild){

					// Delete all its parents (only one parent node: Op ADD)
					for(int j = 0; j < (int)nodes[i].parent.size(); j++){

						// And edges from parent to this node (only one child node: Op MOD)
						oaDpID = mapID[ nodes[i].parent[j] ];
						for(int k = 0; k < (int)nodes[oaDpID].child.size(); k++){
							if(nodes[oaDpID].child[k].nodeID == nodes[i].nodeID){
								nodes[oaDpID].child.erase( nodes[oaDpID].child.begin() + k);
								break;
							}
						}
						nodes[i].parent.erase( nodes[i].parent.begin() + j );
						j--;
					}
				}

				else{
					// Mark back into as un-finished
					operations[ nodes[i].nodeIndex ].isFinished = false;
				}
			}
		}
	}
	
	// For each resource node
	for(int i = 0; i < (int)nodes.size(); i++){

		// Switch
		if(nodes[i].nodeType == RES_SWITCH){

			// All needed resource
			int need = 0;
			for(int j = 0; j < (int)nodes[i].child.size(); j++)
				need += nodes[i].child[j].intWeight;

			// TCAM usage is enough for all links
			int index = nodes[i].nodeIndex;
			if(switches[index].tcamUsage >= need){
				switches[index].tcamUsage -= need;

				// Also delete the links from Operation to Switch
				for(int j = 0; j < (int)nodes[i].child.size(); j++){
					int child = mapID[ nodes[i].child[j].nodeID ];
					for(int k = 0; k < (int)nodes[child].parent.size(); k++){
						if(nodes[child].parent[k] == nodes[i].nodeID){
							nodes[child].parent.erase(nodes[child].parent.begin()+k);
							break;
						}
					}
				}
				nodes[i].child.clear();
			}
		}

		// Links
		if(nodes[i].nodeType == RES_LINK){

			// All needed resource
			double need = 0;
			for(int j = 0; j < (int)nodes[i].child.size(); j++)
				need += nodes[i].child[j].dobWeight;

			// Link usage is enough for all links
			int index = nodes[i].nodeIndex;
			if(links[index].linkCapacity >= need){
				links[index].linkCapacity -= need;

				// Also delete the links from Path to Link
				for(int j = 0; j < (int)nodes[i].child.size(); j++){
					int child = mapID[ nodes[i].child[j].nodeID ];
					for(int k = 0; k < (int)nodes[child].parent.size(); k++){
						if(nodes[child].parent[k] == nodes[i].nodeID){
							nodes[child].parent.erase(nodes[child].parent.begin()+k);
							break;
						}
					}
				}
				nodes[i].child.clear();
			}
		}
	}
}

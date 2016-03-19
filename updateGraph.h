#ifndef UPDATE_GRAPH_H
#define UPDATE_GRAPH_H

// Headers
#include "header.h"
#include "structure.h"

// Update dependency graph
void updateGraph(DPGraph &dpGraph, vector<Switch> &switches, 
		vector<Link> &links, vector<Operation> &operations){

	// For each finished operation node
	for(int i = 0; i < dpGraph.nodes.size(); i++){
		if(dpGraph.nodes[i].nodeType == OPERATION
				&& operations[ dpGraph.nodes[i].nodeIndex ].isFinished){

			// ...
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
				switches[index].tcamUsage -= need;

				// Delete all out links
				for(int j = 0; j < dpGraph.nodes[i].child.size(); j++){
					int child = dpGraph.nodes[i].child[j].nodeID;
					for(int k = 0; k < dpGraph.nodes[child].parent.size(); k++){
						if(dpGraph.nodes[child].parent[k] == index){
							dpGraph.nodes[child].parent.erase(dpGraph.nodes[child].parent.begin()+k);
							break;
						}
					}
				}
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
				links[index].linkCapacity -= need;

				// Delete all out links
				for(int j = 0; j < dpGraph.nodes[i].child.size(); j++){
					int child = dpGraph.nodes[i].child[j].nodeID;
					for(int k = 0; k < dpGraph.nodes[child].parent.size(); k++){
						if(dpGraph.nodes[child].parent[k] == index){
							dpGraph.nodes[child].parent.erase(dpGraph.nodes[child].parent.begin()+k);
							break;
						}
					}
				}
				dpGraph.nodes[i].child.clear();
			}
		}
	}
}

#endif

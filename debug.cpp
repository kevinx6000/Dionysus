// Header
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Debugging function
void Dionysus::debug(void){

	// Links
	#ifdef LINKS
	for(int fID = 0; fID < (int)allFlow.size(); fID++){
		fprintf(stderr, "Flow %d:\n", fID);
		for(int i = 0; i < (int)links.size(); i++)
			fprintf(stderr, "%d to %d, initial = %.2lf\n", links[i].sourceID, links[i].destinationID, links[i].curTraffic[fID]);
	}
	
	#endif

	// Flow
	#ifdef FLOW
	for(int i = 0; i < (int)allFlow.size(); i++){
		fprintf(stderr, "Flow %d: (ingress = %d, path = %d)\n", i, allFlow[i].ingressID, (int)allFlow[i].flowPath.size());
		for(int j = 0; j < (int)allFlow[i].flowPath.size(); j++){
			fprintf(stderr, "\tPath %d: (traffic = %.2lf)\n", j, allFlow[i].flowPath[j].traffic);
			for(int k = 0; k < 2; k++){
				fprintf(stderr, "\t\t");
				fprintf(stderr, k ? "Final" : "Initial");
				fprintf(stderr, "\n\t\t");
				for(int l = 0; l < (int)allFlow[i].flowPath[j].link[k].size();l++)
					fprintf(stderr, " %d-%d", allFlow[i].flowPath[j].link[k][l].sourceID, allFlow[i].flowPath[j].link[k][l].destinationID);
				fprintf(stderr, "\n");
			}
		}
	}
	#endif

	// Dependency graph
	#ifdef GRAPH
	int ind;
	for(int i = 0; i < (int)nodes.size(); i++){
		if(nodes[i].child.size() == 0 && nodes[i].parent.size() == 0) continue;
		fprintf(stderr, "Node id = %2d, type = ", nodes[i].nodeID);
		switch(nodes[i].nodeType){
			// Switch
			case RES_SWITCH:
				ind = nodes[i].nodeIndex;
				fprintf(stderr, "RES_SWITCH, ");
				fprintf(stderr, "Switch ID = %d", switches[ind].switchID);
				break;
			// Link
			case RES_LINK:
				ind = nodes[i].nodeIndex;
				fprintf(stderr, "RES_LINK, ");
				fprintf(stderr, "Link (%d, %d)", links[ind].sourceID, links[ind].destinationID);
				break;
			// Path
			case PATH:
				ind = nodes[i].nodeIndex;
				fprintf(stderr, "PATH");
				fprintf(stderr, ", committed = %.2lf", paths[ind].committed);
				fprintf(stderr, ", avaiable = %.2lf", paths[ind].available);
				break;
			// Operation
			case OPERATION:
				ind = nodes[i].nodeIndex;
				fprintf(stderr, "OPERATION, ");
				fprintf(stderr, "Operation type = ");
				switch(operations[ind].operationType){
					case OP_ADD:
						fprintf(stderr, "ADD");
						break;
					case OP_MOD:
						fprintf(stderr, "MOD");
						break;
					case OP_DEL:
						fprintf(stderr, "DEL");
						break;
					default:
						fprintf(stderr, "OP ERROR");
				}
				break;
			// ???
			default:
				fprintf(stderr, "ERROR");
		}
		fprintf(stderr, ", Children:");
		for(int j = 0; j < (int)nodes[i].child.size(); j++){
			int nid = nodes[i].child[j].nodeID;
			fprintf(stderr, " %d", nid);
			switch(nodes[nid].nodeType){
				case RES_SWITCH:
					fprintf(stderr, "(%d)", nodes[i].child[j].intWeight);
					break;
				case RES_LINK:
					fprintf(stderr, "(%.2lf)", nodes[i].child[j].dobWeight);
					break;
				case PATH:
					if(nodes[i].nodeType == RES_LINK)
						fprintf(stderr, "(%.2lf)", nodes[i].child[j].dobWeight);
					break;
				case OPERATION:
					if(nodes[i].nodeType == RES_SWITCH)
						fprintf(stderr, "(%d)", nodes[i].child[j].intWeight);
					break;
				default:
					break;
			}
		}
		fprintf(stderr, ", Parent:");
		for(int j = 0; j < (int)nodes[i].parent.size(); j++){
			int nid = nodes[i].parent[j];
			fprintf(stderr, " %d", nid);
			switch(nodes[nid].nodeType){
				case RES_SWITCH:
					fprintf(stderr, "-Switch");
					break;
				case RES_LINK:
					fprintf(stderr, "-Link");
					break;
				case PATH:
					fprintf(stderr, "-Path");
					break;
				case OPERATION:
					fprintf(stderr, "-Operation");
					break;
				default:
					break;
			}
		}
		fprintf(stderr, "\n");
	}
	#endif

	// CPL
	#ifdef CALCPL
	for(int i = 0; i < (int)nodes.size(); i++){
		fprintf(stderr, "Node ID = %d, CPL = %d\n", nodes[i].nodeID, nodes[i].CPL);
	}
	#endif
}

#include "header.h"
#include "structure.h"
#include "function.h"

// Switch nodes
vector<Switch>switches;

// Link nodes
vector<Link>links;

// Path nodes
vector<Path>paths;

// Operation nodes
vector<Operation>operations;

// Flow distributions
vector<Flow>flows;

// Dependency graph
DPGraph dpGraph;

// Main function
int main(void){
	
	// Read topology
	readTopo(switches, links);
	
	// Read traffic distribution
	readFlow(flows, switches);
	
	// Create dependency graph
	genDependencyGraph(dpGraph, flows, switches, links, paths, operations);

	// Update dependency graph
	updateGraph(dpGraph, switches, links, paths, operations);

	// CPL without resource links deletion
	if(!calculateCPL(dpGraph)) fprintf(stderr, "Cycle exists\n");
	else{
		// Sort CPL in non-increasing order
		sort(dpGraph.nodes.begin(), dpGraph.nodes.end(), cmpCPL);

		// Rearrange the relative index of dependency nodes
		for(int i = 0; i < dpGraph.nodes.size(); i++)
			dpGraph.mapID[ dpGraph.nodes[i].nodeID ] = i;

		// Check all unscheduled operation nodes
		for(int i = 0; i < dpGraph.nodes.size(); i++){

			// Pick up operation node
			if(dpGraph.nodes[i].nodeType == OPERATION){

				// which is unfinished & can be schedule now
				if(!operations[ dpGraph.nodes[i].nodeIndex ].isFinished &&
					canScheduleOperation(dpGraph, switches, links, paths, operations, dpGraph.mapID[ dpGraph.nodes[i].nodeID ]))
					printf("Node %d can be scheduled now.\n", dpGraph.nodes[i].nodeID);
			}
		}

#ifdef DCPL
		// DEBUG: print CPL
		for(int i = 0; i < dpGraph.nodes.size(); i++)
			printf("Node %2d : %2d\n", dpGraph.nodes[i].nodeID, dpGraph.nodes[i].CPL);
#endif
	}

	// Debug: topology
#ifdef DTOPO
	for(int i = 0; i < switches.size(); i++){
		printf("Switch ID = %d\n", switches[i].switchID);
		for(int j = 0; j < switches[i].port.size(); j++)
			printf(" %d(%d)", switches[i].port[j], switches[i].linkID[j]);
		printf("\n");
	}
#endif

	// Debug: flows
#ifdef DFLOW
	for(int i = 0; i < flows.size(); i++){
		printf("Flow %d:\n", i);
		printf("ingress switch = %d\n", flows[i].ingressID);
		/* TRAFFIC VOLUME REQUIRED */
		for(int d = 0; d < 2; d++){
			printf(d%2 ? "\t<After>\n" : "\t<Before>\n");
			for(int s = 0; s < flows[i].traffic[d].size(); s++){
				printf("\t\tSwitch ID = %d", flows[i].traffic[d][s].switchID);
				for(int r = 0; r < flows[i].traffic[d][s].volume.size(); r++)
					printf(" %.2lf", flows[i].traffic[d][s].volume[r]);
				printf("\n");
			}
		}
	}
#endif

	// Debug: dependency graph
#ifdef DGRAPH
	int ind;
	for(int i = 0; i < dpGraph.nodes.size(); i++){
		printf("Node id = %2d, type = ", dpGraph.nodes[i].nodeID);
		switch(dpGraph.nodes[i].nodeType){
			// Switch
			case RES_SWITCH:
				ind = dpGraph.nodes[i].nodeIndex;
				printf("RES_SWITCH, ");
				printf("Switch ID = %d", switches[ind].switchID);
//				printf("TCAM usage = %d\n", switches[ind].tcamUsage);
				break;
			// Link
			case RES_LINK:
				ind = dpGraph.nodes[i].nodeIndex;
				printf("RES_LINK, ");
				printf("Link (%d, %d)", links[ind].sourceID, links[ind].destinationID);
//				printf("\tLink capacity = %.2lf\n", links[ind].linkCapacity);
				break;
			// Path
			case PATH:
				ind = dpGraph.nodes[i].nodeIndex;
				printf("PATH");
				printf(", committed = %.2lf", paths[ind].committed);
				printf(", avaiable = %.2lf", paths[ind].available);
				break;
			// Operation
			case OPERATION:
				ind = dpGraph.nodes[i].nodeIndex;
				printf("OPERATION, ");
				printf("Operation type = ");
				switch(operations[ind].operationType){
					case OP_ADD:
						printf("ADD");
						break;
					case OP_MOD:
						printf("MOD");
						break;
					case OP_DEL:
						printf("DEL");
						break;
					default:
						printf("OP ERROR");
				}
				break;
			// ???
			default:
				printf("ERROR");
		}
		printf(", Children:");
		for(int j = 0; j < dpGraph.nodes[i].child.size(); j++){
			int nid = dpGraph.nodes[i].child[j].nodeID;
			printf(" %d", nid);
			switch(dpGraph.nodes[nid].nodeType){
				case RES_SWITCH:
					printf("(%d)", dpGraph.nodes[i].child[j].intWeight);
					break;
				case RES_LINK:
					printf("(%.2lf)", dpGraph.nodes[i].child[j].dobWeight);
					break;
				case PATH:
					if(dpGraph.nodes[i].nodeType == RES_LINK)
						printf("(%.2lf)", dpGraph.nodes[i].child[j].dobWeight);
					break;
				case OPERATION:
					if(dpGraph.nodes[i].nodeType == RES_SWITCH)
						printf("(%d)", dpGraph.nodes[i].child[j].intWeight);
					break;
				default:
					break;
			}
		}
		printf(", Parent:");
		for(int j = 0; j < dpGraph.nodes[i].parent.size(); j++){
			int nid = dpGraph.nodes[i].parent[j];
			printf(" %d", nid);
			switch(dpGraph.nodes[nid].nodeType){
				case RES_SWITCH:
					printf("-Switch");
					break;
				case RES_LINK:
					printf("-Link");
					break;
				case PATH:
					printf("-Path");
					break;
				case OPERATION:
					printf("-Operation");
					break;
				default:
					break;
			}
		}
		printf("\n");
	}
#endif

	return 0;
}

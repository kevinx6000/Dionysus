// Header
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Debugging function
void Dionysus::debug(void){

	// Flow
	#ifdef FLOW
	for(int i = 0; i < (int)allFlow.size(); i++){
		printf("Flow %d: (ingress = %d, path = %d)\n", i, allFlow[i].ingressID, (int)allFlow[i].flowPath.size());
		for(int j = 0; j < (int)allFlow[i].flowPath.size(); j++){
			printf("\tPath %d: (traffic = %.2lf)\n", j, allFlow[i].flowPath[j].traffic);
			for(int k = 0; k < 2; k++){
				printf("\t\t");
				printf(k ? "Final" : "Initial");
				printf("\n\t\t");
				for(int l = 0; l < (int)allFlow[i].flowPath[j].link[k].size();l++){
					printf(" %d-%d", allFlow[i].flowPath[j].link[k][l].sourceID, allFlow[i].flowPath[j].link[k][l].destinationID);
				}
				printf("\n");
			}
		}
	}
	#endif
}

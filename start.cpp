// Headers
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Start scheduling
void Dionysus::start(void){

	// Update dependency graph
	updateGraph();
	
	// Cycle exits when calculating CPL
	if(!calculateCPL()) fprintf(stderr, "ERROR: cycle exists\n");
	else{

		// Sort with their CPL
		sortCPL();

		// Check if some operation is ready for scheduled
		for(int i = 0; i < (int)nodes.size(); i++)
			if(canSchedule(mapID[ nodes[i].nodeID ]))
				fprintf(stderr, "Node %d is ready for scheduled.\n", nodes[i].nodeID);
	}
}

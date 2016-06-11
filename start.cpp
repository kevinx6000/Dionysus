// Headers
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Start scheduling
void Dionysus::start(void){

	// Variable
	int owID;
	bool isScheduled;
	bool isDeadlock;

	// Initialize counters
	addRuleCnt = 0;
	modRuleCnt = 0;
	delRuleCnt = 0;
	roundCnt = 0;

	// Run until all update is finished
	while(true){

		// Update dependency graph
		updateGraph();

		// Cycle exits when calculating CPL
		calculateCPL();
//		if(!calculateCPL())
//			fprintf(stderr, "ERROR: cycle exists\n");
//			fprintf(stderr, "WARNING: cycle exists, random CPL assigned.\n");

		// Sort with their CPL
		sortCPL();

		// Check if some operation is ready for scheduled
		isScheduled = false;
		for(int i = 0; i < (int)nodes.size(); i++)
			if(canSchedule(mapID[ nodes[i].nodeID ])){
				schedule(mapID[ nodes[i].nodeID ]);
				isScheduled = true;
			}

		// No operation scheduled
		if(!isScheduled){

			// Check deadlock
			isDeadlock = false;
			for(int i = 0; i < (int)nodes.size(); i++){
				if(nodes[i].nodeType == OPERATION){
					owID = nodes[i].nodeIndex;
					if(operations[owID].operationType == OP_MOD &&
							!operations[owID].isFinished){
						isDeadlock = true;
						break;
					}
				}
			}

			// Deadlock
			if(isDeadlock){
				fprintf(stderr, "DEADLOCK occurs!\n");
				debug();
				exit(1);
			}

			// Finished
			else{
//				fprintf(stderr, "All operation finished.\n");
				break;
			}
		}

		// Round count
		roundCnt ++;
	}
}

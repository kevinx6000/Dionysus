// Headers
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Start scheduling
void Dionysus::start(void){

	// Variable
	int owID;
	int numOfPlan;
	bool isScheduled;
	bool isDeadlock;

	// Check cycling/chaining effect
	if(checkCompete()){
		numOfPlan = 2;

		// Sort the hops of new transition plan
		for(int fID = 0; fID < (int)newFlow[1].size(); fID++){
			for(int pID = 0; pID < (int)newFlow[1][fID].flowPath.size(); pID++){
				sort(newFlow[1][fID].flowPath[pID].link[0].begin(), newFlow[1][fID].flowPath[pID].link[0].end(), cmpPATH);
				sort(newFlow[1][fID].flowPath[pID].link[1].begin(), newFlow[1][fID].flowPath[pID].link[1].end(), cmpPATH);
			}
		}
	}
	else{
		numOfPlan = 1;
		newFlow[0] = allFlow;
	}

	// Transition plan(s)
	for(int plan = 0; plan < numOfPlan; plan++){

		// Update initial resource usage
		initResource(newFlow[plan]);

		// Generate dependency graph
		genDepGraph(newFlow[plan]);

		// Run until all update is finished
		while(true){

			// Update dependency graph
			updateGraph();

			// Cycle exits when calculating CPL
			if(!calculateCPL())
				fprintf(stderr, "WARNING: cycle exists, random CPL assigned.\n");

			// Sort with their CPL
			sortCPL();

			// Check if some operation is ready for scheduled
			isScheduled = false;
			for(int i = 0; i < (int)nodes.size(); i++)
				if(canSchedule(mapID[ nodes[i].nodeID ], newFlow[plan])){
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
					fprintf(stderr, "All operation finished.\n");
					break;
				}
			}
		}

		// Reset the resource
		reset();
	}
}

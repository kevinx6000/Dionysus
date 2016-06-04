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

	// Initialize counters
	addRuleCnt = 0;
	modRuleCnt = 0;
	delRuleCnt = 0;

	// Check cycling/chaining effect
	if(checkCompete()){
		numOfPlan = 2;

		// Sort the hops of new transition plan
		for(int state = 0; state < 2; state++){
			for(int fID = 0; fID < (int)newFlow[state].size(); fID++){
				for(int pID = 0; pID < (int)newFlow[state][fID].flowPath.size(); pID++){
					sort(newFlow[state][fID].flowPath[pID].link[0].begin(), newFlow[state][fID].flowPath[pID].link[0].end(), cmpPATH);
					sort(newFlow[state][fID].flowPath[pID].link[1].begin(), newFlow[state][fID].flowPath[pID].link[1].end(), cmpPATH);
				}
			}
		}
		
		// DEBUG
		/*
		for(int state = 0; state < 2; state++){
			fprintf(stderr, "[Trans%d]:\n", state+1);
			for(int flowID = 0; flowID < (int)newFlow[state].size(); flowID++){
				fprintf(stderr, "Flow %d:\n", newFlow[state][flowID].flowID);
				for(int pathID = 0; pathID < (int)newFlow[state][flowID].flowPath.size(); pathID++){
					fprintf(stderr, "\tPath %d:\n\tI:", pathID);
					for(int hop = 0; hop < (int)newFlow[state][flowID].flowPath[pathID].link[0].size(); hop++)
						fprintf(stderr, " %d-%d", newFlow[state][flowID].flowPath[pathID].link[0][hop].sourceID, newFlow[state][flowID].flowPath[pathID].link[0][hop].destinationID);
					fprintf(stderr, "\n\tF:");
					for(int hop = 0; hop < (int)newFlow[state][flowID].flowPath[pathID].link[1].size(); hop++)
						fprintf(stderr, " %d-%d", newFlow[state][flowID].flowPath[pathID].link[1][hop].sourceID, newFlow[state][flowID].flowPath[pathID].link[1][hop].destinationID);
					fprintf(stderr, "\n");
				}
			}
		}
		*/
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

// Header
#include "header.h"
#include "structure.h"
#include "dionysus.h"
#include "compete.h"

// Check if chaining/Cycling exist
bool Dionysus::checkCompete(void){

	// Variable
	bool hasCompete = false;
	vector<Flow>newFlow1, newFlow2;
	
	// Initialize compete graph checker
	this->compete.initialize(links, trancNode, interNode, pod);

	// Update initial remaining resource
	this->compete.updateResource(allFlow);

	// Record release and require resource list
	this->compete.flowChangeList(allFlow);

	// Create compete graph
	this->compete.createGraph(allFlow);

	// Check cycle
	if(this->compete.checkCycle()){
		fprintf(stderr, "Cycle found\n");

		// Change old plan to new plan
		this->compete.changePlan(links, allFlow, newFlow1, newFlow2, pod);

		// DEBUG
		fprintf(stderr, "[Trans1]:\n");
		for(int flowID = 0; flowID < (int)newFlow1.size(); flowID++){
			fprintf(stderr, "Flow %d:\n", newFlow1[flowID].flowID);
			for(int pathID = 0; pathID < (int)newFlow1[flowID].flowPath.size(); pathID++){
				fprintf(stderr, "\tPath %d:\n\tI:", pathID);
				for(int hop = 0; hop < (int)newFlow1[flowID].flowPath[pathID].link[0].size(); hop++)
					fprintf(stderr, " %d-%d", newFlow1[flowID].flowPath[pathID].link[0][hop].sourceID, newFlow1[flowID].flowPath[pathID].link[0][hop].destinationID);
				fprintf(stderr, "\n\tF:");
				for(int hop = 0; hop < (int)newFlow1[flowID].flowPath[pathID].link[1].size(); hop++)
					fprintf(stderr, " %d-%d", newFlow1[flowID].flowPath[pathID].link[1][hop].sourceID, newFlow1[flowID].flowPath[pathID].link[1][hop].destinationID);
				fprintf(stderr, "\n");
			}
		}
		fprintf(stderr, "[Trans2]:\n");
		for(int flowID = 0; flowID < (int)newFlow2.size(); flowID++){
			fprintf(stderr, "Flow %d:\n", newFlow2[flowID].flowID);
			for(int pathID = 0; pathID < (int)newFlow2[flowID].flowPath.size(); pathID++){
				fprintf(stderr, "\tPath %d:\n\tI:", pathID);
				for(int hop = 0; hop < (int)newFlow2[flowID].flowPath[pathID].link[0].size(); hop++)
					fprintf(stderr, " %d-%d", newFlow2[flowID].flowPath[pathID].link[0][hop].sourceID, newFlow2[flowID].flowPath[pathID].link[0][hop].destinationID);
				fprintf(stderr, "\n\tF:");
				for(int hop = 0; hop < (int)newFlow2[flowID].flowPath[pathID].link[1].size(); hop++)
					fprintf(stderr, " %d-%d", newFlow2[flowID].flowPath[pathID].link[1][hop].sourceID, newFlow2[flowID].flowPath[pathID].link[1][hop].destinationID);
				fprintf(stderr, "\n");
			}
		}

	}

	return hasCompete;
}

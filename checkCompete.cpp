// Header
#include "header.h"
#include "structure.h"
#include "dionysus.h"
#include "compete.h"

// Check if chaining/Cycling exist
bool Dionysus::checkCompete(void){

	// Variable
	bool hasCompete = false;
	
	// Initialize compete graph checker
	this->compete.initialize(links, trancNode, interNode, pod);

	// Update initial remaining resource
	this->compete.updateResource(allFlow);

	// Record release and require resource list
	this->compete.flowChangeList(allFlow);

	// Create compete graph
	this->compete.createGraph(allFlow);

	// Check cycle
	if(this->compete.needTemp()){
		hasCompete = true;
		fprintf(stderr, "Cycle found\n");

		// Change old plan to new plan
		this->compete.changePlan(links, allFlow, newFlow[0], newFlow[1], pod);

		// DEBUG
		fprintf(stderr, "[Trans1]:\n");
		for(int flowID = 0; flowID < (int)newFlow[0].size(); flowID++){
			fprintf(stderr, "Flow %d:\n", newFlow[0][flowID].flowID);
			for(int pathID = 0; pathID < (int)newFlow[0][flowID].flowPath.size(); pathID++){
				fprintf(stderr, "\tPath %d:\n\tI:", pathID);
				for(int hop = 0; hop < (int)newFlow[0][flowID].flowPath[pathID].link[0].size(); hop++)
					fprintf(stderr, " %d-%d", newFlow[0][flowID].flowPath[pathID].link[0][hop].sourceID, newFlow[0][flowID].flowPath[pathID].link[0][hop].destinationID);
				fprintf(stderr, "\n\tF:");
				for(int hop = 0; hop < (int)newFlow[0][flowID].flowPath[pathID].link[1].size(); hop++)
					fprintf(stderr, " %d-%d", newFlow[0][flowID].flowPath[pathID].link[1][hop].sourceID, newFlow[0][flowID].flowPath[pathID].link[1][hop].destinationID);
				fprintf(stderr, "\n");
			}
		}
		fprintf(stderr, "[Trans2]:\n");
		for(int flowID = 0; flowID < (int)newFlow[1].size(); flowID++){
			fprintf(stderr, "Flow %d:\n", newFlow[1][flowID].flowID);
			for(int pathID = 0; pathID < (int)newFlow[1][flowID].flowPath.size(); pathID++){
				fprintf(stderr, "\tPath %d:\n\tI:", pathID);
				for(int hop = 0; hop < (int)newFlow[1][flowID].flowPath[pathID].link[0].size(); hop++)
					fprintf(stderr, " %d-%d", newFlow[1][flowID].flowPath[pathID].link[0][hop].sourceID, newFlow[1][flowID].flowPath[pathID].link[0][hop].destinationID);
				fprintf(stderr, "\n\tF:");
				for(int hop = 0; hop < (int)newFlow[1][flowID].flowPath[pathID].link[1].size(); hop++)
					fprintf(stderr, " %d-%d", newFlow[1][flowID].flowPath[pathID].link[1][hop].sourceID, newFlow[1][flowID].flowPath[pathID].link[1][hop].destinationID);
				fprintf(stderr, "\n");
			}
		}
	}

	return hasCompete;
}

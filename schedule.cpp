// Header
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Schedule operation Oi
void Dionysus::schedule(int owDpID){

	// Variable
	int oaID, owID, odID, fID;
	int oaDpID, odDpID;

	// Update according to rulesets inside operation nodes
	if(operations[ nodes[owDpID].nodeIndex ].ruleSet.size() > 0){

		// Flow ID
		fID = operations[ nodes[owDpID].nodeIndex ].flowID;
		fprintf(stderr, "Flow %d:\n", fID);

		// Add rules
		oaDpID = mapID[ nodes[owDpID].parent[0] ];
		oaID = nodes[oaDpID].nodeIndex;
		for(int i = 0; i < (int)operations[oaID].ruleSet.size(); i++){
			fprintf(stderr, "Add rules @ switch %d:", operations[oaID].ruleSet[i].switchID);
			for(int j = 0; j < (int)operations[oaID].ruleSet[i].traffic.size(); j++)
				fprintf(stderr, " %.2lf", operations[oaID].ruleSet[i].traffic[j]);
			fprintf(stderr, "\n");
		}

		// Modify rules
		owID = nodes[owDpID].nodeIndex;
		fprintf(stderr, "Modify rule @ ingress switch %d:", operations[owID].ruleSet[0].switchID);
		for(int i = 0; i < (int)operations[owID].ruleSet[0].traffic.size(); i++)
			fprintf(stderr, " %.2lf", operations[owID].ruleSet[0].traffic[i]);
		fprintf(stderr, "\n");

		// Delete rules
		odDpID = -1;
		for(int i = 0; i < (int)nodes[owDpID].child.size(); i++)
			if(nodes[ mapID[ nodes[owDpID].child[i].nodeID ] ].nodeType == OPERATION){
				odDpID = mapID[ nodes[owDpID].child[i].nodeID ];
				break;
			}
		odID = nodes[odDpID].nodeIndex;
		for(int i = 0; i < (int)operations[odID].ruleSet.size(); i++)
			fprintf(stderr, "Delete rules @ switch %d\n", operations[odID].ruleSet[i].switchID);

		// Mark three operation nodes as finished
		operations[oaDpID].isFinished = true;
		operations[owDpID].isFinished = true;
		operations[odDpID].isFinished = true;
	}

	// No update needed
	else{
		fprintf(stderr, "No update needed\n");
	}
}

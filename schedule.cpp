// Header
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Schedule operation Oi
void Dionysus::schedule(int owDpID){

	// Variable
//	int fTag;
	int oaID, owID, odID;
	int oaDpID, odDpID;
	double totTraffic;

	// Update according to rulesets inside operation nodes
	if(operations[ nodes[owDpID].nodeIndex ].ruleSet.size() > 0){

		// Flow Tag
//		fTag = operations[ nodes[owDpID].nodeIndex ].flowTag;
//		fprintf(stderr, "Flow %d:\n", fTag);

		// Add rules
		oaDpID = mapID[ nodes[owDpID].parent[0] ];
		oaID = nodes[oaDpID].nodeIndex;
		for(int i = 0; i < (int)operations[oaID].ruleSet.size(); i++){
//			fprintf(stderr, "Add rules @ switch %d:", operations[oaID].ruleSet[i].switchID);
			totTraffic = 0.0;
			for(int j = 0; j < (int)operations[oaID].ruleSet[i].traffic.size(); j++)
				totTraffic += operations[oaID].ruleSet[i].traffic[j];
//			for(int j = 0; j < (int)operations[oaID].ruleSet[i].traffic.size(); j++)
//				fprintf(stderr, " %.2lf", operations[oaID].ruleSet[i].traffic[j]/totTraffic);
//			fprintf(stderr, "\n");
			addRuleCnt++;
		}

		// Modify rules
		owID = nodes[owDpID].nodeIndex;
//		fprintf(stderr, "Modify rule @ ingress switch %d:", operations[owID].ruleSet[0].switchID);
		totTraffic = 0.0;
		for(int i = 0; i < (int)operations[owID].ruleSet[0].traffic.size(); i++)
			totTraffic += operations[owID].ruleSet[0].traffic[i];
//		for(int i = 0; i < (int)operations[owID].ruleSet[0].traffic.size(); i++)
//			fprintf(stderr, " %.2lf", operations[owID].ruleSet[0].traffic[i]/totTraffic);
//		fprintf(stderr, "\n");
		modRuleCnt++;

		// Delete rules
		odDpID = -1;
		for(int i = 0; i < (int)nodes[owDpID].child.size(); i++)
			if(nodes[ mapID[ nodes[owDpID].child[i].nodeID ] ].nodeType == OPERATION){
				odDpID = mapID[ nodes[owDpID].child[i].nodeID ];
				break;
			}
		odID = nodes[odDpID].nodeIndex;
		for(int i = 0; i < (int)operations[odID].ruleSet.size(); i++){
//			fprintf(stderr, "Delete rules @ switch %d\n", operations[odID].ruleSet[i].switchID);
			delRuleCnt++;
		}

		// Mark three operation nodes as finished
		operations[oaID].isFinished = true;
		operations[owID].isFinished = true;
		operations[odID].isFinished = true;
	}

	// No update needed
	else{
		fprintf(stderr, "No update needed\n");
	}
}

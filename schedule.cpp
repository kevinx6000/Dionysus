// Header
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Schedule operation Oi
void Dionysus::schedule(int owDpID){

	// Variable
	int pDpID, pID, sID, dID, ingID;
	int fID, fpID, portID;
	bool isChanged, isZero;
	double traffic;
	vector<double>preTraffic;

	// Record the traffic distribution of this flow before update on all links
	fID = operations[ nodes[owDpID].nodeIndex ].flowID;
	for(int i = 0; i < (int)links.size(); i++)
		preTraffic.push_back(links[i].curTraffic[fID]);

	// Update all path with new traffic distribution
	for(int i = 0; i < (int)nodes[owDpID].child.size(); i++){
		pDpID = mapID[ nodes[owDpID].child[i].nodeID ];
		if(nodes[pDpID].nodeType == PATH){
			pID = nodes[pDpID].nodeIndex;

			// Corresponding flow
			fpID = paths[pID].flowPathID;
			traffic = paths[pID].committed;

			// New switches along the paths
			for(int j = 0; j < (int)allFlow[fID].flowPath[fpID].link[1].size(); j++){
				sID = allFlow[fID].flowPath[fpID].link[1][j].sourceID;
				dID = allFlow[fID].flowPath[fpID].link[1][j].destinationID;
				portID = findDstPort(sID, dID);

				// Update current state with committed traffic
				if(portID != -1)
					links[ switches[sID].linkID[portID] ].curTraffic[fID] += traffic;

				// Exception: no such ID
				else{
					fprintf(stderr, "No such switch ID (%d) exisits from %d\n", dID, sID);
					exit(1);
				}
			}

			// Old switches along the paths
			for(int j = 0; j < (int)allFlow[fID].flowPath[fpID].link[0].size(); j++){
				sID = allFlow[fID].flowPath[fpID].link[0][j].sourceID;
				dID = allFlow[fID].flowPath[fpID].link[0][j].destinationID;
				portID = findDstPort(sID, dID);

				// Update current state with committed traffic + create new rule
				if(portID != -1)
					links[ switches[sID].linkID[portID] ].curTraffic[fID] -= traffic;

				// Exception: no such ID
				else{
					fprintf(stderr, "No such switch ID (%d) exisits from %d\n", dID, sID);
					exit(1);
				}
			}

		}
	}

	// Ingress switch ID
	ingID = operations[ nodes[owDpID].nodeIndex ].switchID;

	// Add rule with new version
	for(int i = 0; i < (int)switches.size(); i++){
		if(i != ingID){

			// Check if links are changed OR all links are zero
			isZero = true, isChanged = false;
			for(int j = 0; j < (int)switches[i].linkID.size(); j++){
				traffic = links[ switches[i].linkID[j] ].curTraffic[fID];
				if(traffic != 0.0) isZero = false;
				if(traffic != preTraffic[ switches[i].linkID[j] ]) isChanged = true;
			}

			// Install new rule if: 
			// 1. Link traffic of at least one link is not zero 
			// 2. Link traffic of at least one link has changed
			if(!isZero && isChanged){
				fprintf(stderr, "Install new rule @ switch %d:", i);
				for(int j = 0; j < (int)switches[i].linkID.size(); j++)
					fprintf(stderr, " %.2lf", links[ switches[i].linkID[j] ].curTraffic[fID]);
				fprintf(stderr, "\n");
			}
		}
	}

	// Modify ratio @ ingress switch
	fprintf(stderr, "Modify rule @ ingress switch %d:", ingID);
	for(int i = 0; i < (int)switches[ingID].linkID.size(); i++)
		fprintf(stderr, " %.2lf", links[ switches[ingID].linkID[i] ].curTraffic[fID]);
	fprintf(stderr, "\n");

	// Delete rule with old version
	for(int i = 0; i < (int)switches.size(); i++){
		if(i != ingID){

			// Check if previous traffic is zero for all links
			isZero = true, isChanged = false;
			for(int j = 0; j < (int)switches[i].linkID.size(); j++){
				traffic = preTraffic[ switches[i].linkID[j] ];
				if(traffic != 0.0) isZero = false;
				if(traffic != links[ switches[i].linkID[j] ].curTraffic[fID]) isChanged = true;
			}

			// Delete rule if
			// 1. Previous link traffic of at least one link is not zero
			// 2. Link traffic of at least one link has changed
			if(!isZero && isChanged)
				fprintf(stderr, "Delete old rule @ switch %d\n", i);
		}
	}
}

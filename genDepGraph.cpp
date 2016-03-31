// Header
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Generate dependency graph
void Dionysus::genDepGraph(void){

	// Variables
	int pInDpID, pOutDpID, sID1, sID2, dID1, dID2;
	int ptr1, ptr2, siz1, siz2;
	double traffic;
	Path ptmp;

	// Resource Node: switch
	for(int i = 0; i < (int)switches.size(); i++)
		switches[i].dpID = createNode(RES_SWITCH, i);

	// Resource Node: link
	for(int i = 0; i < (int)links.size(); i++)
		links[i].dpID = createNode(RES_LINK, i);

	// For each flow
	for(int i = 0; i < (int)allFlow.size(); i++){

		// For each path
		for(int j = 0; j < (int)allFlow[i].flowPath.size(); j++){

			// Record traffic size
			traffic = allFlow[i].flowPath[j].traffic;

			// Create path nodes
			ptmp.dpID = pInDpID  = createNode(PATH, paths.size());
			paths.push_back(ptmp);
			ptmp.dpID = pOutDpID = createNode(PATH, paths.size());
			paths.push_back(ptmp);

			// Compare initial & final distribution
			ptr1 = ptr2 = 0;
			siz1 = allFlow[i].flowPath[j].link[0].size();
			siz2 = allFlow[i].flowPath[j].link[1].size();
			while(ptr1 < siz1 && ptr2 < siz2){

				// Switch ID for two pointers
				sID1 = allFlow[i].flowPath[j].link[0][ptr1].sourceID;
				dID1 = allFlow[i].flowPath[j].link[0][ptr1].destinationID;
				sID2 = allFlow[i].flowPath[j].link[1][ptr2].sourceID;
				dID2 = allFlow[i].flowPath[j].link[1][ptr2].destinationID;

				// Ingress switch
				if(sID1 == allFlow[i].ingressID){
					// TODO: handle ingress switch
					ptr1++;
					continue;
				}
				if(sID2 == allFlow[i].ingressID){
					// TODO: handle ingress switch
					ptr2++;
					continue;
				}

				// Same switch
				if(sID1 == sID2){

					// New port
					if(dID1 != dID2){
						
						// Require capacity on sID2-dID2
						requireCap(sID2, dID2, pInDpID, traffic);

						// Release capacity on sID1-dID1
//						releaseCap(switches, links, sID1, dID1);
					}

					// Update pointer
					ptr1++;
					ptr2++;
				}

				// Add switch sID2: sID1 > sID2
				else if(sID1 > sID2){

					// Require capacity on sID2-dID2
					requireCap(sID2, dID2, pInDpID, traffic);

					// Update pointer
					ptr2++;
				}

				// Del switch sID1: sID1 < sID2
				else{

					// Release capacity on sID1-dID1
//					releaseCap(switches, links, sID1, dID1);

					// Update pointer
					ptr1++;
				}
			}
		}
	}

}

// Require capacity @ link: sID-dID
void Dionysus::requireCap(int sID, int dID, int pInDpID, double traffic){

	// Find out destination port
	int portID = findDstPort(sID, dID);

	// No such port
	if(portID == -1){
		fprintf(stderr, "Error: ID %d not found @ switch %d\n", dID, sID);
		exit(1);
	}

	// Add link
	else{

		// Link resourceID
		int rID = links[ switches[sID].port[portID] ].dpID;

		// Add dependency link: Link -> Path
		Edge etmp;
		etmp.nodeID = pInDpID;
		etmp.dobWeight = traffic;
		nodes[rID].child.push_back(etmp);
		nodes[pInDpID].parent.push_back(rID);
	}
}

// Release capacity @ link: sID-dID
void Dionysus::releaseCap(int sID, int dID, int pOutDpID, double traffic){

	// Find out destination port
	int portID = findDstPort(sID, dID);

	// No such port
	if(portID == -1){
		fprintf(stderr, "Error: ID %d not found @ switch %d\n", dID, sID);
		exit(1);
	}

	// Add link
	else{

		// Link resourceID
		int rID = links[ switches[sID].port[portID] ].dpID;

		// Add dependency link: Path -> Link
		Edge etmp;
		etmp.nodeID = rID;
		etmp.dobWeight = traffic;
		nodes[pOutDpID].child.push_back(etmp);
		nodes[rID].parent.push_back(pOutDpID);
	}
}
// Find out port of destination switch dID @ switch sID
int Dionysus::findDstPort(int sID, int dID){
	for(int i = 0; i < (int)switches[sID].port.size(); i++)
		if(switches[sID].port[i] == dID) return i;
	return -1;
}

// Create Node
int Dionysus::createNode(int nodeType, int nodeIndex = 0){
	Node ntmp;
	ntmp.nodeID = (int)this->nodes.size();
	ntmp.nodeType = nodeType;
	ntmp.nodeIndex = nodeIndex;
	ntmp.CPL = -1;
	this->nodes.push_back(ntmp);
	this->mapID.push_back(ntmp.nodeID);
	return ntmp.nodeID;
}

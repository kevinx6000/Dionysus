// Header
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Generate dependency graph
void Dionysus::genDepGraph(void){

	// Variables
	int pInDpID, pOutDpID, sID1, sID2, dID1, dID2, owDpID, ingPtr1, ingPtr2;
	int ptr1, ptr2, siz1, siz2;
	bool hasDiff;
	double traffic;
	Path ptmp;
	Edge etmp;
	Operation otmp;

	// Resource Node: switch
	for(int i = 0; i < (int)switches.size(); i++)
		switches[i].dpID = createNode(RES_SWITCH, i);

	// Resource Node: link
	for(int i = 0; i < (int)links.size(); i++)
		links[i].dpID = createNode(RES_LINK, i);

	// For each flow
	for(int i = 0; i < (int)allFlow.size(); i++){

		// Clear ADD/DEL node vector
		addOpList.clear();
		delOpList.clear();

		// Create Op MOD node
		otmp.dpID = owDpID = createNode(OPERATION, operations.size());
		otmp.operationType = OP_MOD;
		otmp.switchID = allFlow[i].ingressID;
		otmp.isFinished = false;
		operations.push_back(otmp);

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
			hasDiff = false;
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
					ingPtr1 = ptr1++;
					continue;
				}
				if(sID2 == allFlow[i].ingressID){
					ingPtr2 = ptr2++;
					continue;
				}

				// Same switch
				if(sID1 == sID2){

					// New port
					if(dID1 != dID2){
						
						// Require capacity on sID2-dID2
						requireCap(sID2, dID2, pInDpID, traffic);
						insertOpADD(sID2, pInDpID, traffic);

						// Release capacity on sID1-dID1
						releaseCap(sID1, dID1, pOutDpID, traffic);
						insertOpDEL(sID1);
						hasDiff = true;
					}
					ptr1++;
					ptr2++;
				}

				// Add switch sID2: sID1 > sID2
				else if(sID1 > sID2){

					// Require capacity on sID2-dID2
					requireCap(sID2, dID2, pInDpID, traffic);
					insertOpADD(sID2, pInDpID, traffic);

					/* Note: only the first path flow do not need this node (check @ scheduling) */
					insertOpDEL(sID2);
					hasDiff = true;
					ptr2++;
				}

				// Del switch sID1: sID1 < sID2
				else{

					// Release capacity on sID1-dID1
					releaseCap(sID1, dID1, pOutDpID, traffic);
					insertOpDEL(sID1);

					/* Note: only the last path flow do not need this node (check @ scheduling) */
					insertOpADD(sID1, pInDpID);
					hasDiff = true;
					ptr1++;
				}
			}

			// Old rule need to be deleted
			while(ptr1 < siz1){

				// Switch ID
				sID1 = allFlow[i].flowPath[j].link[0][ptr1].sourceID;
				dID1 = allFlow[i].flowPath[j].link[0][ptr1].destinationID;

				// Release capacity on sID1-dID1
				releaseCap(sID1, dID1, pOutDpID, traffic);
				insertOpDEL(sID1);
				
				/* Note: only the last path flow do not need this node (check @ scheduling) */
				insertOpADD(sID1, pInDpID);
				hasDiff = true;
				ptr1++;
			}

			// New rule need to be added
			while(ptr2 < siz2){

				// Switch ID
				sID2 = allFlow[i].flowPath[j].link[1][ptr2].sourceID;
				dID2 = allFlow[i].flowPath[j].link[1][ptr2].destinationID;

				// Require capacity on sID2-dID2
				requireCap(sID2, dID2, pInDpID, traffic);
				insertOpADD(sID2, pInDpID, traffic);

				/* Note: only the first path flow do not need this node (check @ scheduling) */
				insertOpDEL(sID2);
				hasDiff = true;
				ptr2++;
			}

			// Ingress switch traffic difference
			sID1 = allFlow[i].ingressID;
			dID1 = allFlow[i].flowPath[j].link[0][ingPtr1].destinationID;
			dID2 = allFlow[i].flowPath[j].link[1][ingPtr2].destinationID;
			if(dID1 != dID2){

				// Require capacity on ingressID-dID2
				requireCap(sID1, dID2, pInDpID, traffic);

				// Release capacity on ingressID-dID1
				releaseCap(sID1, dID1, pOutDpID, traffic);
				hasDiff = true;
			}

			// Add link dependency: Operation MOD -> PathOut
			if(hasDiff){
				etmp.nodeID = pOutDpID;
				nodes[owDpID].child.push_back(etmp);
				nodes[pOutDpID].parent.push_back(owDpID);
			}
		}

		// Add link dependency: Operation ADD -> Operation MOD
		for(int j = 0; j < (int)addOpList.size(); j++){
			etmp.nodeID = owDpID;
			nodes[addOpList[j]].child.push_back(etmp);
			nodes[owDpID].parent.push_back(addOpList[j]);
		}

		// Add link dependency: Operation MOD -> Operation DEL
		for(int j = 0; j < (int)delOpList.size(); j++){
			etmp.nodeID = delOpList[j];
			nodes[owDpID].child.push_back(etmp);
			nodes[delOpList[j]].parent.push_back(owDpID);
		}
	}
}

// Require capacity @ link: sID-dID
void Dionysus::requireCap(int sID, int dID, int pInDpID, double traffic){

	// Variables
	int portID, rID;
	Edge etmp;

	// Find out destination port
	portID = findDstPort(sID, dID);

	// No such port
	if(portID == -1){
		fprintf(stderr, "Error: ID %d not found @ switch %d\n", dID, sID);
		exit(1);
	}

	// Add link
	else{

		// Link resourceID
		rID = links[ switches[sID].linkID[portID] ].dpID;

		// Add dependency link: Link -> Path
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
	Edge etmp;

	// No such port
	if(portID == -1){
		fprintf(stderr, "Error: ID %d not found @ switch %d\n", dID, sID);
		exit(1);
	}

	// Add link
	else{

		// Link resourceID
		int rID = links[ switches[sID].linkID[portID] ].dpID;

		// Add dependency link: Path -> Link
		etmp.nodeID = rID;
		etmp.dobWeight = traffic;
		nodes[pOutDpID].child.push_back(etmp);
		nodes[rID].parent.push_back(pOutDpID);
	}
}

// Insert OP_ADD node into dependency graph
void Dionysus::insertOpADD(int sID, int pInDpID, double traffic){

	// Variables
	Edge etmp;
	Operation otmp;

	// Add operation node: OP_ADD @ sID
	otmp.dpID = createNode(OPERATION, operations.size());
	otmp.operationType = OP_ADD;
	otmp.switchID = sID;
	otmp.isFinished = false;
	operations.push_back(otmp);
	addOpList.push_back(otmp.dpID);

	// Add dependency link: Switch -> Operation
	etmp.nodeID = otmp.dpID;
	etmp.intWeight = 1;
	nodes[switches[sID].dpID].child.push_back(etmp);
	nodes[otmp.dpID].parent.push_back(switches[sID].dpID);

	// Add dependency link: Path -> Operation
	etmp.nodeID = otmp.dpID;
	etmp.dobWeight = traffic;
	nodes[pInDpID].child.push_back(etmp);
	nodes[otmp.dpID].parent.push_back(pInDpID);
}

// Insert OP_DEL node into dependency graph
void Dionysus::insertOpDEL(int sID){

	// Variables
	Edge etmp;
	Operation otmp;

	// Add operation node: OP_DEL @ sID
	otmp.dpID = createNode(OPERATION, operations.size());
	otmp.operationType = OP_DEL;
	otmp.switchID = sID;
	otmp.isFinished = false;
	operations.push_back(otmp);
	delOpList.push_back(otmp.dpID);

	// Add dependency link: Operation -> Switch
	etmp.nodeID = switches[sID].dpID;
	etmp.intWeight = 1;
	nodes[otmp.dpID].child.push_back(etmp);
	nodes[switches[sID].dpID].parent.push_back(otmp.dpID);
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

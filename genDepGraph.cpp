// Header
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Generate dependency graph
void Dionysus::genDepGraph(void){

	// Variables
	int pInDpID, pOutDpID, sID1, sID2, dID1, dID2, owDpID, oaDpID, odDpID, ingPtr1, ingPtr2;
	int ptr1, ptr2, siz1, siz2;
	bool hasDiff, exist;
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

		// Create Op ADD node
		otmp.dpID = oaDpID = createNode(OPERATION, operations.size());
		otmp.operationType = OP_ADD;
		otmp.isFinished = false;
		operations.push_back(otmp);

		// Create Op MOD node
		otmp.dpID = owDpID = createNode(OPERATION, operations.size());
		otmp.operationType = OP_MOD;
		otmp.switchID = allFlow[i].ingressID;
		otmp.flowID = i;
		otmp.isFinished = false;
		operations.push_back(otmp);

		// Create Op DEL node
		otmp.dpID = odDpID = createNode(OPERATION, operations.size());
		otmp.operationType = OP_DEL;
		otmp.isFinished = false;
		operations.push_back(otmp);

		// Add dependency link: Op ADD -> Op MOD
		etmp.nodeID = owDpID;
		nodes[oaDpID].child.push_back(etmp);
		nodes[owDpID].parent.push_back(oaDpID);

		// Add dependency link: Op MOD -> Op DEL
		etmp.nodeID = odDpID;
		nodes[owDpID].child.push_back(etmp);
		nodes[odDpID].parent.push_back(owDpID);

		// For each path
		for(int j = 0; j < (int)allFlow[i].flowPath.size(); j++){

			// Sort path links according to their ID
			sort(allFlow[i].flowPath[j].link[0].begin(), allFlow[i].flowPath[j].link[0].end(), cmpPATH);
			sort(allFlow[i].flowPath[j].link[1].begin(), allFlow[i].flowPath[j].link[1].end(), cmpPATH);

			// Record traffic size
			traffic = allFlow[i].flowPath[j].traffic;

			// Create path nodes
			ptmp.dpID = pInDpID  = createNode(PATH, paths.size());
			ptmp.flowPathID = j;
			paths.push_back(ptmp);
			ptmp.dpID = pOutDpID = createNode(PATH, paths.size());
			ptmp.flowPathID = j;
			paths.push_back(ptmp);

			// Pair ID
			paths[ nodes[pInDpID].nodeIndex ].pairID = nodes[pOutDpID].nodeIndex;
			paths[ nodes[pOutDpID].nodeIndex ].pairID = nodes[pInDpID].nodeIndex;

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
						
						// Require/Release capacity on sID2-dID2/sID1-dID1
						requireCap(sID2, dID2, pInDpID, traffic);
						releaseCap(sID1, dID1, pOutDpID, traffic);

						// Check if Switch -> Op ADD aleady exists
						exist = false;
						for(int k = 0; k < (int)nodes[switches[sID1].dpID].child.size(); k++){
							if(nodes[switches[sID1].dpID].child[k].nodeID == oaDpID){
								exist = true;
								nodes[switches[sID1].dpID].child[k].intWeight++;
								break;
							}
						}

						// Add dependency link: Switch -> Op ADD
						if(!exist){
							etmp.nodeID = oaDpID;
							etmp.intWeight = 1;
							/* Fake link weight: we update switch resource according to actual expense */
							nodes[switches[sID1].dpID].child.push_back(etmp);
							nodes[oaDpID].parent.push_back(switches[sID1].dpID);
						}

						// Check if Op DEL -> Switch aleady exists
						exist = false;
						for(int k = 0; k < (int)nodes[odDpID].child.size(); k++){
							if(nodes[odDpID].child[k].nodeID == switches[sID1].dpID){
								exist = true;
								nodes[odDpID].child[k].intWeight++;
								break;
							}
						}

						// Add dependency link: Op DEL -> Switch
						if(!exist){
							etmp.nodeID = switches[sID1].dpID;
							etmp.intWeight = 1;
							/* Fake link weight: we update switch resource according to actual expense */
							nodes[odDpID].child.push_back(etmp);
							nodes[switches[sID1].dpID].parent.push_back(odDpID);
						}
						hasDiff = true;
					}
					ptr1++;
					ptr2++;
				}

				// Add switch sID2: sID1 > sID2
				else if(sID1 > sID2){

					// Require capacity on sID2-dID2
					requireCap(sID2, dID2, pInDpID, traffic);

					// Check if Switch -> Op ADD aleady exists
					exist = false;
					for(int k = 0; k < (int)nodes[switches[sID2].dpID].child.size(); k++){
						if(nodes[switches[sID2].dpID].child[k].nodeID == oaDpID){
							exist = true;
							nodes[switches[sID2].dpID].child[k].intWeight++;
							break;
						}
					}

					// Add dependency link: Switch -> Op ADD
					if(!exist){
						etmp.nodeID = oaDpID;
						etmp.intWeight = 1;
						nodes[switches[sID2].dpID].child.push_back(etmp);
						nodes[oaDpID].parent.push_back(switches[sID2].dpID);
					}

					// Check if Op DEL -> Switch already exists
					exist = false;
					for(int k = 0; k < (int)nodes[odDpID].child.size(); k++){
						if(nodes[odDpID].child[k].nodeID == switches[sID2].dpID){
							exist = true;
							nodes[odDpID].child[k].intWeight++;
							break;
						}
					}

					// Add dependency link: Op DEL -> Switch
					if(!exist){
						etmp.nodeID = switches[sID2].dpID;
						etmp.intWeight = 1;
						nodes[odDpID].child.push_back(etmp);
						nodes[switches[sID2].dpID].parent.push_back(odDpID);
					}

					hasDiff = true;
					ptr2++;
				}

				// Del switch sID1: sID1 < sID2
				else{

					// Release capacity on sID1-dID1
					releaseCap(sID1, dID1, pOutDpID, traffic);

					// Check if Switch -> Op ADD aleady exists
					exist = false;
					for(int k = 0; k < (int)nodes[switches[sID1].dpID].child.size(); k++){
						if(nodes[switches[sID1].dpID].child[k].nodeID == oaDpID){
							exist = true;
							nodes[switches[sID1].dpID].child[k].intWeight++;
							break;
						}
					}

					// Add dependency link: Switch -> Op ADD
					if(!exist){
						etmp.nodeID = oaDpID;
						etmp.intWeight = 1;
						/* Fake link weight: we update switch resource according to actual expense */
						nodes[switches[sID1].dpID].child.push_back(etmp);
						nodes[oaDpID].parent.push_back(switches[sID1].dpID);
					}

					// Check if Op DEL -> Switch already exists
					exist = false;
					for(int k = 0; k < (int)nodes[odDpID].child.size(); k++){
						if(nodes[odDpID].child[k].nodeID == switches[sID1].dpID){
							exist = true;
							nodes[odDpID].child[k].intWeight++;
							break;
						}
					}

					// Add dependency link: Op DEL -> Switch
					if(!exist){
						etmp.nodeID = switches[sID1].dpID;
						etmp.intWeight = 1;
						/* Fake link weight: we update switch resource according to actual expense */
						nodes[odDpID].child.push_back(etmp);
						nodes[switches[sID1].dpID].parent.push_back(odDpID);
					}

					hasDiff = true;
					ptr1++;
				}
			}

			// Old rule need to be deleted
			while(ptr1 < siz1){

				// Switch ID
				sID1 = allFlow[i].flowPath[j].link[0][ptr1].sourceID;
				dID1 = allFlow[i].flowPath[j].link[0][ptr1].destinationID;

				// Ingress switch
				if(sID1 == allFlow[i].ingressID){
					ingPtr1 = ptr1++;
					continue;
				}
			
				// Release capacity on sID1-dID1
				releaseCap(sID1, dID1, pOutDpID, traffic);

				// Check if Switch -> Op ADD aleady exists
				exist = false;
				for(int k = 0; k < (int)nodes[switches[sID1].dpID].child.size(); k++){
					if(nodes[switches[sID1].dpID].child[k].nodeID == oaDpID){
						exist = true;
						nodes[switches[sID1].dpID].child[k].intWeight++;
						break;
					}
				}

				// Add dependency link: Switch -> Op ADD
				if(!exist){
					etmp.nodeID = oaDpID;
					etmp.intWeight = 1;
					/* Fake link weight: we update switch resource according to actual expense */
					nodes[switches[sID1].dpID].child.push_back(etmp);
					nodes[oaDpID].parent.push_back(switches[sID1].dpID);
				}

				// Check if Op DEL -> Switch already exists
				exist = false;
				for(int k = 0; k < (int)nodes[odDpID].child.size(); k++){
					if(nodes[odDpID].child[k].nodeID == switches[sID1].dpID){
						exist = true;
						nodes[odDpID].child[k].intWeight++;
						break;
					}
				}

				// Add dependency link: Op DEL -> Switch
				if(!exist){
					etmp.nodeID = switches[sID1].dpID;
					etmp.intWeight = 1;
					/* Fake link weight: we update switch resource according to actual expense */
					nodes[odDpID].child.push_back(etmp);
					nodes[switches[sID1].dpID].parent.push_back(odDpID);
				}

				hasDiff = true;
				ptr1++;
			}

			// New rule need to be added
			while(ptr2 < siz2){

				// Switch ID
				sID2 = allFlow[i].flowPath[j].link[1][ptr2].sourceID;
				dID2 = allFlow[i].flowPath[j].link[1][ptr2].destinationID;

				// Ingress switch
				if(sID2 == allFlow[i].ingressID){
					ingPtr2 = ptr2++;
					continue;
				}

				// Require capacity on sID2-dID2
				requireCap(sID2, dID2, pInDpID, traffic);

				// Check if Switch -> Op ADD aleady exists
				exist = false;
				for(int k = 0; k < (int)nodes[switches[sID2].dpID].child.size(); k++){
					if(nodes[switches[sID2].dpID].child[k].nodeID == oaDpID){
						exist = true;
						nodes[switches[sID2].dpID].child[k].intWeight++;
						break;
					}
				}

				// Add dependency link: Switch -> Op ADD
				if(!exist){
					etmp.nodeID = oaDpID;
					etmp.intWeight = 1;
					/* Fake link weight: we update switch resource according to actual expense */
					nodes[switches[sID2].dpID].child.push_back(etmp);
					nodes[oaDpID].parent.push_back(switches[sID2].dpID);
				}

				// Check if Op DEL -> Switch already exists
				exist = false;
				for(int k = 0; k < (int)nodes[odDpID].child.size(); k++){
					if(nodes[odDpID].child[k].nodeID == switches[sID2].dpID){
						exist = true;
						nodes[odDpID].child[k].intWeight++;
						break;
					}
				}

				// Add dependency link: Op DEL -> Switch
				if(!exist){
					etmp.nodeID = switches[sID2].dpID;
					etmp.intWeight = 1;
					/* Fake link weight: we update switch resource according to actual expense */
					nodes[odDpID].child.push_back(etmp);
					nodes[switches[sID2].dpID].parent.push_back(odDpID);
				}

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

			// Traffic has difference
			if(hasDiff){

				// Add dependency link: PathIn -> Op ADD
				etmp.nodeID = oaDpID;
				etmp.dobWeight = traffic;
				nodes[pInDpID].child.push_back(etmp);
				nodes[oaDpID].parent.push_back(pInDpID);

				// Add dependency link: Op MOD -> PathOut
				etmp.nodeID = pOutDpID;
				/* Weight?? */
				nodes[owDpID].child.push_back(etmp);
				nodes[pOutDpID].parent.push_back(owDpID);
			}
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

// Sort comparison function for Node with CPL
bool Dionysus::cmpPATH(Link A, Link B){
	return A.sourceID < B.sourceID;
}

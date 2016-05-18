// Header
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Initialize resource usage
void Dionysus::initResource(const vector<Flow>& flowPlan){

	// Initialize resource
	for(int i = 0; i < (int)switches.size(); i++)
		switches[i].tcamUsage = TCAM_CAPACITY;
	for(int i = 0; i < (int)trancNode.size(); i++)
		trancNode[i].nodeCapacity = LINK_CAPACITY;
	for(int i = 0; i < (int)interNode.size(); i++)
		interNode[i].nodeCapacity = LINK_CAPACITY;
	for(int i = 0; i < (int)links.size(); i++){
		links[i].linkCapacity = LINK_CAPACITY;
		links[i].curTraffic.clear();
	}

	// For each flow
	for(int flowID = 0; flowID < (int)flowPlan.size(); flowID++){

		// Initial link traffic = 0
		for(int i = 0; i < (int)links.size(); i++)
			links[i].curTraffic.push_back(0.0);

		// For each path flow
		for(int pathID = 0; pathID < (int)flowPlan[flowID].flowPath.size(); pathID++){
			double traffic = flowPlan[flowID].flowPath[pathID].traffic;

			// Initial traffic only (link[0])
			for(int hop = 0; hop < (int)flowPlan[flowID].flowPath[pathID].link[0].size(); hop++){

				// Update this link with initial traffic
				int srcID = flowPlan[flowID].flowPath[pathID].link[0][hop].sourceID;
				int dstID = flowPlan[flowID].flowPath[pathID].link[0][hop].destinationID;
				int portID = findDstPort(srcID, dstID);
				if(portID != -1){

					int linkID = switches[srcID].linkID[portID];
					links[linkID].linkCapacity -= traffic;
					links[linkID].curTraffic[flowID] += traffic;

					// Wireless link
					if(links[linkID].isWireless){

						// Wireless AP capacity
						trancNode[ switches[srcID].trancID ].nodeCapacity -= traffic;
						trancNode[ switches[dstID].trancID ].nodeCapacity -= traffic;

						// Interference
						for(int ii = 0; ii < (int)links[linkID].iList.size(); ii++)
							interNode[ switches[ links[linkID].iList[ii] ].interID ].nodeCapacity -= traffic;
					}
				}

				// Exception
				else{
					fprintf(stderr, "Error: no such port exists.\n");
					exit(1);
				}
			}
		}
	}
}

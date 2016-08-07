// Header
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Read topology
void Dionysus::readTopo(void){

	// Variables
	int k;
	int n;
	int numOfCore;
	int numOfAggr;
	int numOfEdge;
	int src;
	int dst;
	Switch stmp;
	Link ltmp;

	// Fattree index K
	scanf("%d", &k);
	if(k < 2 || k % 2 ){
		fprintf(stderr, "Error: illegal K as input.\n");
		exit(1);
	}

	// Number of nodes
	numOfCore = (k/2)*(k/2);
	numOfAggr = k*k/2;
	numOfEdge = k*k/2;
	n = numOfCore + numOfAggr + numOfEdge;

	// Initial value of TCAM and Link capacity
	stmp.tcamUsage = TCAM_CAPACITY;
	ltmp.linkCapacity = LINK_CAPACITY;

	// Create switches
	for(int i = 0; i < n; i++){

		// Initialize
		stmp.switchID = i;
		switches.push_back(stmp);
	}

	// Link: Core - Aggregate
	for(int i = 0; i < numOfAggr; i++){
		src = numOfCore + i;
		for(int j = 0; j < k/2; j++){
			dst = (i % (k/2)) * (k/2) + j;

			// Aggr -> Core
			switches[src].port.push_back(dst);
			switches[src].linkID.push_back(links.size());
			ltmp.sourceID = src;
			ltmp.destinationID = dst;
			links.push_back(ltmp);

			// Core -> Aggr
			switches[dst].port.push_back(src);
			switches[dst].linkID.push_back(links.size());
			ltmp.sourceID = dst;
			ltmp.destinationID = src;
			links.push_back(ltmp);
		}
	}

	// Link: Aggregate - Edge
	for(int i = 0; i < numOfAggr; i++){
		src = numOfCore + i;
		for(int j = 0; j < k/2; j++){
			dst = numOfCore + numOfAggr + (i / (k/2)) * (k/2) + j;

			// Aggr -> Edge
			switches[src].port.push_back(dst);
			switches[src].linkID.push_back(links.size());
			ltmp.sourceID = src;
			ltmp.destinationID = dst;
			links.push_back(ltmp);

			// Edge -> Aggr
			switches[dst].port.push_back(src);
			switches[dst].linkID.push_back(links.size());
			ltmp.sourceID = dst;
			ltmp.destinationID = src;
			links.push_back(ltmp);
		}
	}
}

// Read flow
void Dionysus::readFlow(void){
	
	// Variables
	int i;
	int numFlow;
	int numPath;
	int numLink;
	int portID;
	int linkID;
	Flow ftmp;
	FlowPath ptmp;
	Link ltmp;
	
	// For each flow
	scanf("%d", &numFlow);
	for(int fID = 0; fID < numFlow; fID++){

		// Initialize link traffic of every flow
		for(i = 0; i < (int)links.size(); i++)
			links[i].curTraffic.push_back(0.0);

		// Ingress switch
		scanf("%d", &ftmp.ingressID);

		// Number of paths
		scanf("%d", &numPath);
		while(numPath--){

			// Traffic volume
			scanf("%lf", &ptmp.traffic);

			// Initial & Final path
			for(i = 0; i < 2; i++){

				// Number of links
				scanf("%d", &numLink);
				while(numLink--){

					// Source to destination
					scanf("%d%d", &ltmp.sourceID, &ltmp.destinationID);

					// Initial traffic 
					if(!i){

						// Update this link with initial traffic
						portID = findDstPort(ltmp.sourceID, ltmp.destinationID);
						if(portID != -1){
							linkID = switches[ltmp.sourceID].linkID[portID];
							links[linkID].linkCapacity -= ptmp.traffic;
							links[linkID].curTraffic[fID] += ptmp.traffic;
						}

						// Exception
						else{
							fprintf(stderr, "Error: no such port exists.\n");
							exit(1);
						}
					}

					// Record the path
					ptmp.link[i].push_back(ltmp);
				}
			}
			ftmp.flowPath.push_back(ptmp);
			for(i = 0; i < 2; i++) ptmp.link[i].clear();
		}
		allFlow.push_back(ftmp);
		ftmp.flowPath.clear();
	}
}


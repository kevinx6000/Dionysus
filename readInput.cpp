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
	int mid;
	double widSw;
	double lenSw;
	double x1, y1, x2, y2;
	Switch stmp;
	Link ltmp;
	TrancNode ttmp;
	InterNode itmp;

	// Constants
	const double feet = 0.3048;
	const double inch = 0.0254;

	// Width and length of switch
	widSw = 24*inch;
	lenSw = 48*inch;

	// Fattree index K
	scanf("%d", &k);
	if(k < 2 || k % 2 ){
		fprintf(stderr, "Error: illegal K as input.\n");
		exit(1);
	}
	pod = k;

	// Number of nodes
	numOfCore = (k/2)*(k/2);
	numOfAggr = k*k/2;
	numOfEdge = k*k/2;
	n = numOfCore + numOfAggr + numOfEdge;

	// Initial value of TCAM and Link capacity
	// Wired link
	stmp.tcamUsage = TCAM_CAPACITY;
	ltmp.linkCapacity = LINK_CAPACITY;
	ltmp.isWireless = false;
	ttmp.nodeCapacity = LINK_CAPACITY;
	itmp.nodeCapacity = LINK_CAPACITY;

	// Create switches
	for(int i = 0; i < n; i++){

		// Initialize
		stmp.switchID = i;
		stmp.tcamUsage = TCAM_CAPACITY;
		switches.push_back(stmp);
	}

	// Positions for ToR switches
	for(int i = 0; i < numOfEdge; i++){
		switches[numOfCore + numOfAggr + i].posXY[0] = (i % (k/2))*widSw + 0.5*widSw + ((i / (k/2)) % 4) * (10 * feet + (k/2) * widSw);
		switches[numOfCore + numOfAggr + i].posXY[1] = 0.5*lenSw + (i / (k*4/2)) * (lenSw + 8*feet);
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

	// Link: Edge - Edge
	// Wireless link
	ltmp.isWireless = true;
	for(int i = 0; i < numOfEdge; i++){
		src = numOfCore + numOfAggr + i;
		for(int j = 0; j < numOfEdge; j++){
			dst = numOfCore + numOfAggr + j;
			if(src == dst) continue;

			// Distance
			x1 = switches[src].posXY[0];
			y1 = switches[src].posXY[1];
			x2 = switches[dst].posXY[0];
			y2 = switches[dst].posXY[1];
			if(dis(x1, y1, x2, y2) <= WIRELESS_RANGE){

				// Src -> Dst
				switches[src].port.push_back(dst);
				switches[src].linkID.push_back(links.size());
				ltmp.sourceID = src;
				ltmp.destinationID = dst;

				// Interference list
				for(int z = 0; z < numOfEdge; z++){
					mid = numOfCore + numOfAggr + z;
					if(src == mid) continue;

					// Position and vector operation
					if(vecdot(switches[src].posXY, switches[dst].posXY, switches[src].posXY, switches[mid].posXY) > 0 &&
						vecdot(switches[src].posXY, switches[dst].posXY, switches[mid].posXY, switches[dst].posXY) >= 0 &&
						vecdis(switches[src].posXY, switches[dst].posXY, switches[src].posXY, switches[mid].posXY) <= 11*inch){
						ltmp.iList.push_back(mid);
					}
				}
				links.push_back(ltmp);
				ltmp.iList.clear();
			}
		}

		// Transceiver and interference node
		switches[src].trancID = trancNode.size();
		ttmp.switchID = src;
		trancNode.push_back(ttmp);
		switches[src].interID = interNode.size();
		itmp.switchID = src;
		interNode.push_back(itmp);
	}
}

// Read flow
void Dionysus::readFlow(void){
	
	// Variables
	int i;
	int numFlow;
	int numPath;
	int numLink;
	Flow ftmp;
	FlowPath ptmp;
	Link ltmp;
	
	// For each flow
	scanf("%d", &numFlow);
	for(int fID = 0; fID < numFlow; fID++){

		// Ingress switch
		scanf("%d", &ftmp.ingressID);

		// Assign flow ID
		ftmp.flowID = fID;
		ftmp.flowTag= fID;

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

					// Record the path
					ptmp.link[i].push_back(ltmp);
				}

				// Record destination switch ID
				ptmp.dstID[i] = ptmp.link[i][ ptmp.link[i].size()-1 ].destinationID;

				// Sort the link ID non-decreasingly
				sort(ptmp.link[i].begin(), ptmp.link[i].end(), cmpPATH);
			}
			ftmp.flowPath.push_back(ptmp);
			for(i = 0; i < 2; i++) ptmp.link[i].clear();
		}
		allFlow.push_back(ftmp);
		ftmp.flowPath.clear();
	}
}


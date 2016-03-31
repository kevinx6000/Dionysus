// Header
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Read topology
void Dionysus::readTopo(void){

	// Variables
	int n;
	int numPort;
	int nxt;
	Switch stmp;
	Link ltmp;

	// For each switch
	scanf("%d", &n);
	for(int id = 0; id < n; id++){

		// Initialize
		stmp.switchID = id;
		stmp.tcamUsage = TCAM_CAPACITY;
		ltmp.sourceID = id;
		ltmp.linkCapacity = LINK_CAPACITY;

		// For each port
		scanf("%d", &numPort);
		for(int port = 0; port < numPort; port++){
			
			// The switch ID of this port connects to
			scanf("%d", &nxt);
			stmp.port.push_back(nxt);
			stmp.linkID.push_back(links.size());

			// Link node
			ltmp.destinationID = nxt;
			links.push_back(ltmp);
		}
		switches.push_back(stmp);
		stmp.port.clear();
		stmp.linkID.clear();
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
	while(numFlow--){

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


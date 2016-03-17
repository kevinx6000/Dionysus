#ifndef READ_TOPO_H
#define READ_TOPO_H

// Headers
#include "header.h"
#include "structure.h"

// Read Topology
void readTopo(vector<Switch> &switches, vector<Link> &links){

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

#endif

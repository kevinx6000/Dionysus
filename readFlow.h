#ifndef READ_FLOW_H
#define READ_FLOW_H

// Headers
#include "header.h"
#include "structure.h"

// Read Flow
void readFlow(vector<Flow> &allFlow){
	
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

#endif

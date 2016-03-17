#ifndef READ_FLOW_H
#define READ_FLOW_H

// Headers
#include "header.h"
#include "structure.h"

// Read Flow
void readFlow(vector<Flow> &allFlow, const vector<Switch> &switches){

	// Variables
	int numFlow;
	int numSwitch;
	double volume;
	Flow ftmp;
	SwitchINFO itmp;

	// For each flow
	scanf("%d", &numFlow);
	while(numFlow--){

		// Ingress switch
		scanf("%d", &ftmp.ingressID);

		// Initial and Final distribution
		for(int d = 0; d < 2; d++){

			// For each switch
			scanf("%d", &numSwitch);
			for(int sw = 0; sw < numSwitch; sw++){

				// Switch ID
				scanf("%d", &itmp.switchID);

				// Ratio of each link
				for(int lk = 0; lk < switches[itmp.switchID].port.size(); lk++){
					scanf("%lf", &volume);
					itmp.volume.push_back(volume);
				}
				ftmp.traffic[d].push_back(itmp);
				itmp.volume.clear();
			}
		}
		allFlow.push_back(ftmp);
		for(int d = 0; d < 2; d++) ftmp.traffic[d].clear();
	}
}

#endif

// Header
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Create wireless shortest path list
void Dionysus::wirelessPath(void){

	// Variable
	int nowID, nxtID, linkID;
	int numOfCore;
	int numOfAggr;
	int numOfEdge;
	int shift;
	int nxtItf;

	// For SPFA
	int head, tail;
	int que[MAX_EDGE];
	int dis[MAX_EDGE];
	int itf[MAX_EDGE];
	int pre[MAX_EDGE];
	bool inque[MAX_EDGE];
	vector<int>vHop;
	vector<int>vRev;
	vector< vector<int> >vPath;

	// Number of each kind of switches
	numOfCore = (pod/2) * (pod/2);
	numOfAggr = pod * (pod/2);
	numOfEdge = pod * (pod/2);
	shift = numOfCore + numOfAggr;

	// Initialize all pair as empty path
	for(int i = 0; i < numOfEdge; i++)
		wirelessSP.push_back(vPath);

	// All pair wireless shortest path
	for(int i = 0; i < numOfEdge; i++){

		// Initialize
		for(int j = 0; j < numOfEdge; j++){
			dis[j] = INF;
			pre[j] = -1;
			inque[j] = false;
		}
		head = tail = 0;

		// SPFA
		dis[i] = 0;
		itf[i] = 0;
		que[tail++] = i;
		inque[i] = true;
		while(head < tail){
			nowID = que[head % MAX_EDGE];
			inque[nowID] = false;
			head++;

			// Neighbors
			for(int j = 0; j < (int)switches[nowID + shift].port.size(); j++){
				nxtID = switches[nowID + shift].port[j] - shift;
				linkID = switches[nowID + shift].linkID[j];
				if(!links[linkID].isWireless) continue;

				// Shorter distance
				if(dis[nowID] + 1 < dis[nxtID]){
					dis[nxtID] = dis[nowID] + 1;
					itf[nxtID] = itf[nowID] + (int)links[linkID].iList.size();
					pre[nxtID] = nowID;
					if(!inque[nxtID]){
						que[tail % MAX_EDGE] = nxtID;
						inque[nxtID] = true;
						tail++;
					}
				}

				// Equal distance
				else if(dis[nowID] + 1 == dis[nxtID]){
					nxtItf = itf[nowID] + (int)links[linkID].iList.size();
					if(nxtItf < itf[nxtID]){
						itf[nxtID] = nxtItf;
						pre[nxtID] = nowID;
						if(!inque[nxtID]){
							que[tail % MAX_EDGE] = nxtID;
							inque[nxtID] = true;
							tail++;
						}
					}
				}
			}
		}

		// For all destinations
		for(int j = 0; j < numOfEdge; j++){

			// Not self
			if(j != i && pre[j] != -1){

				// Retrieve path
				nowID = j;
				while(nowID != i){
					vRev.push_back(nowID + shift);
					nowID = pre[nowID];
				}
				vRev.push_back(i + shift);

				// Reverse back
				for(int k = vRev.size()-1; k >= 0; k--)
					vHop.push_back(vRev[k]);
			}

			// Record path
			wirelessSP[i].push_back(vHop);
			
			// Clear
			vHop.clear();
			vRev.clear();
		}
	}
}

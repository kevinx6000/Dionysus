// Header
#include "header.h"
#include "structure.h"
#include "compete.h"

// Empty constructor
Compete::Compete(){
	isInitLink = false;
	isInitTranc = false;
	isInitInter = false;
}

// Initializer
void Compete::initialize(const vector<Link>& initLinks, const vector<TrancNode>& initTranc, const vector<InterNode>& initInter, int k){

	// Variables
	int totalNode;
	int srcID, dstID, linkID;
	CompRes ctmp;
	map<int, int>mtmp;

	// Total number of switches (given k)
	totalNode = 5*k*k/4;

	// Pre-insert index mapping
	for(int i = 0; i < totalNode; i++)
		linkMap.push_back(mtmp);

	// Copy links
	for(int i = 0; i < (int)initLinks.size(); i++){
		ctmp.srcID = initLinks[i].sourceID;
		ctmp.dstID = initLinks[i].destinationID;
		ctmp.isWireless = initLinks[i].isWireless;
		ctmp.resCap = LINK_CAPACITY;
		ctmp.resType = LINK_RES;
		this->linkMap[ ctmp.srcID ][ ctmp.dstID ] = this->compRes.size();
		this->compRes.push_back(ctmp);
	}

	// Copy transceiver node
	for(int i = 0; i < (int)initTranc.size(); i++){
		ctmp.srcID = initTranc[i].switchID;
		ctmp.resCap = LINK_CAPACITY;
		ctmp.resType = TRANC_RES;
		this->trancMap[ ctmp.srcID ] = this->compRes.size();
		this->compRes.push_back(ctmp);
	}

	// Copy interference node
	for(int i = 0; i < (int)initInter.size(); i++){
		ctmp.srcID = initInter[i].switchID;
		ctmp.resCap = LINK_CAPACITY;
		ctmp.resType = INTER_RES;
		this->interMap[ ctmp.srcID ] = this->compRes.size();
		this->compRes.push_back(ctmp);
	}

	// Create interference list in terms of the indexing here
	for(int i = 0; i < (int)initLinks.size(); i++){
		srcID = initLinks[i].sourceID;
		dstID = initLinks[i].destinationID;
		linkID = linkMap[srcID][dstID];
		if(compRes[linkID].isWireless)
			for(int j = 0; j < (int)initLinks[i].iList.size(); j++)
				this->compRes[linkID].iList.push_back( interMap[ initLinks[i].iList[j] ] );
	}

	// Set as already initialized
	isInitLink = true;
	isInitTranc = true;
	isInitInter = true;
}

// Constructor
Compete::Compete(const vector<Link>& initLinks, const vector<TrancNode>& initTranc, const vector<InterNode>& initInter, int k){
	this->initialize(initLinks, initTranc, initInter, k);
}

// Update resource
void Compete::updateResource(const vector<Flow>& allFlow){

	// Variables
	int srcID;
	int dstID;
	int linkID;
	double traffic;

	// Update intial resource consumption
	for(int i = 0; i < (int)allFlow.size(); i++){
		for(int j = 0; j < (int)allFlow[i].flowPath.size(); j++){
			traffic = allFlow[i].flowPath[j].traffic;
			for(int k = 0; k < (int)allFlow[i].flowPath[j].link[0].size(); k++){
				srcID = allFlow[i].flowPath[j].link[0][k].sourceID;
				dstID = allFlow[i].flowPath[j].link[0][k].destinationID;
				linkID = linkMap[srcID][dstID];

				// All links
				compRes[linkID].resCap -= traffic;

				// Wireless links
				if(compRes[linkID].isWireless){

					// Transceiver
					compRes[ trancMap[srcID] ].resCap -= traffic;
					compRes[ trancMap[dstID] ].resCap -= traffic;

					// Interference
					for(int z = 0; z < (int)compRes[linkID].iList.size(); z++){
						compRes[ compRes[linkID].iList[z] ].resCap -= traffic;
					}
				}
			}
		}
	}

	// DEBUG
	/*
	for(int i = 0; i < (int)compRes.size(); i++){
		if(compRes[i].resCap != LINK_CAPACITY){
			switch(compRes[i].resType){
				case LINK_RES:
					fprintf(stderr, "Link: %d-%d", compRes[i].srcID, compRes[i].dstID);
					break;
				case TRANC_RES:
					fprintf(stderr, "Tranc: %d", compRes[i].srcID);
					break;
				case INTER_RES:
					fprintf(stderr, "Inter: %d", compRes[i].srcID);
					break;
			}
			fprintf(stderr, " (%.2lf)\n", compRes[i].resCap);
		}
	}
	*/
}

// Calculate the changes (require/release) of each flow
void Compete::flowChangeList(const vector<Flow>& allFlow){

	// Variables
	int siz1, siz2;
	int ptr1, ptr2;
	int ingPtr1, ingPtr2;
	int sID1, sID2, dID1, dID2;
	int linkID, interID;
	int relCnt, reqCnt;
	double traffic;
	FlowPair ftmp;
	set<int>trancSwitch;
	set<int>interSwitch;
	set<int>::iterator setItr;

	// Each path flow
	for(int flowID = 0; flowID < (int)allFlow.size(); flowID++){
		for(int pathID = 0; pathID < (int)allFlow[flowID].flowPath.size(); pathID++){
			ptr1 = ptr2 = 0;
			siz1 = allFlow[flowID].flowPath[pathID].link[0].size();
			siz2 = allFlow[flowID].flowPath[pathID].link[1].size();
			ftmp.flowID = flowID;
			ftmp.pathID = pathID;
			ftmp.traffic = traffic = allFlow[flowID].flowPath[pathID].traffic;

			// Initialize transceiver and interference resource to zero
			trancSwitch.clear();
			interSwitch.clear();
			for(int state = 0; state < 2; state++){
				for(int hop = 0; hop < (int)allFlow[flowID].flowPath[pathID].link[state].size(); hop++){
					sID1 = allFlow[flowID].flowPath[pathID].link[state][hop].sourceID;
					dID1 = allFlow[flowID].flowPath[pathID].link[state][hop].destinationID;
					linkID = linkMap[sID1][dID1];
					if(compRes[linkID].isWireless){
						compRes[ trancMap[sID1] ].relCnt = 0;
						compRes[ trancMap[sID1] ].reqCnt = 0;
						compRes[ trancMap[dID1] ].relCnt = 0;
						compRes[ trancMap[dID1] ].reqCnt = 0;
						trancSwitch.insert(trancMap[sID1]);
						trancSwitch.insert(trancMap[dID1]);
						for(int x = 0; x < (int)compRes[linkID].iList.size(); x++){
							interID = compRes[linkID].iList[x];
							compRes[interID].relCnt = 0;
							compRes[interID].reqCnt = 0;
							interSwitch.insert(interID);
						}
					}
				}
			}

			// Check the difference between initial and final state
			while(ptr1 < siz1 && ptr2 < siz2){
				sID1 = allFlow[flowID].flowPath[pathID].link[0][ptr1].sourceID;
				dID1 = allFlow[flowID].flowPath[pathID].link[0][ptr1].destinationID;
				sID2 = allFlow[flowID].flowPath[pathID].link[1][ptr2].sourceID;
				dID2 = allFlow[flowID].flowPath[pathID].link[1][ptr2].destinationID;
				if(sID1 == allFlow[flowID].ingressID){
					ingPtr1 = ptr1++;
					continue;
				}
				if(sID2 == allFlow[flowID].ingressID){
					ingPtr2 = ptr2++;
					continue;
				}

				// Same switch
				if(sID1 == sID2){
					if(dID1 != dID2){
						compRes[ linkMap[sID1][dID1] ].relList.push_back(ftmp);
						compRes[ linkMap[sID2][dID2] ].reqList.push_back(ftmp);
					}
					ptr1++;
					ptr2++;
				}

				// Add switch sID2
				else if(sID1 > sID2){
					compRes[ linkMap[sID2][dID2] ].reqList.push_back(ftmp);
					ptr2++;
				}

				// Del switch sID1
				else{
					compRes[ linkMap[sID1][dID1] ].relList.push_back(ftmp);
					ptr1++;
				}
			}

			// Remaining old switches
			while(ptr1 < siz1){
				sID1 = allFlow[flowID].flowPath[pathID].link[0][ptr1].sourceID;
				dID1 = allFlow[flowID].flowPath[pathID].link[0][ptr1].destinationID;
				if(sID1 == allFlow[flowID].ingressID){
					ingPtr1 = ptr1++;
					continue;
				}
				compRes[ linkMap[sID1][dID1] ].relList.push_back(ftmp);
				ptr1++;
			}

			// Remaining new switches
			while(ptr2 < siz2){
				sID2 = allFlow[flowID].flowPath[pathID].link[1][ptr2].sourceID;
				dID2 = allFlow[flowID].flowPath[pathID].link[1][ptr2].destinationID;
				if(sID2 == allFlow[flowID].ingressID){
					ingPtr2 = ptr2++;
					continue;
				}
				compRes[ linkMap[sID2][dID2] ].reqList.push_back(ftmp);
				ptr2++;
			}

			// Ingress switch
			sID1 = allFlow[flowID].ingressID;
			dID1 = allFlow[flowID].flowPath[pathID].link[0][ingPtr1].destinationID;
			dID2 = allFlow[flowID].flowPath[pathID].link[1][ingPtr2].destinationID;
			if(dID1 != dID2){
				compRes[ linkMap[sID1][dID1] ].relList.push_back(ftmp);
				compRes[ linkMap[sID1][dID2] ].reqList.push_back(ftmp);
			}

			// Release/require count for transceiver and interference resource
			for(int state = 0; state < 2; state++){
				for(int hop = 0; hop < (int)allFlow[flowID].flowPath[pathID].link[state].size(); hop++){
					sID1 = allFlow[flowID].flowPath[pathID].link[state][hop].sourceID;
					dID1 = allFlow[flowID].flowPath[pathID].link[state][hop].destinationID;
					linkID = linkMap[sID1][dID1];
					if(compRes[linkID].isWireless){

						// Release
						if(state < 1){
							compRes[ trancMap[sID1] ].relCnt ++;
							compRes[ trancMap[dID1] ].relCnt ++;
							for(int x = 0; x < (int)compRes[linkID].iList.size(); x++){
								interID = compRes[linkID].iList[x];
								compRes[interID].relCnt ++;
							}
						}

						// Require
						else{
							compRes[ trancMap[sID1] ].reqCnt ++;
							compRes[ trancMap[dID1] ].reqCnt ++;
							for(int x = 0; x < (int)compRes[linkID].iList.size(); x++){
								interID = compRes[linkID].iList[x];
								compRes[interID].reqCnt ++;
							}
						}
					}
				}
			}

			// Check transceiver/interference resource difference
			for(setItr = trancSwitch.begin(); setItr != trancSwitch.end(); setItr++){
				relCnt = compRes[ *setItr ].relCnt;
				reqCnt = compRes[ *setItr ].reqCnt;
				if(relCnt < reqCnt){
					ftmp.traffic = (reqCnt - relCnt) * traffic;
					compRes[ *setItr ].reqList.push_back(ftmp);
				}
				else if(relCnt > reqCnt){
					ftmp.traffic = (relCnt - reqCnt) * traffic;
					compRes[ *setItr ].relList.push_back(ftmp);
				}
			}
			for(setItr = interSwitch.begin(); setItr != interSwitch.end(); setItr++){
				relCnt = compRes[ *setItr ].relCnt;
				reqCnt = compRes[ *setItr ].reqCnt;
				if(relCnt < reqCnt){
					ftmp.traffic = (reqCnt - relCnt) * traffic;
					compRes[ *setItr ].reqList.push_back(ftmp);
				}
				else if(relCnt > reqCnt){
					ftmp.traffic = (relCnt - reqCnt) * traffic;
					compRes[ *setItr ].relList.push_back(ftmp);
				}
			}
		}
	}

	// DEBUG
	/*
	for(int i = 0; i < (int)compRes.size(); i++){
		if(compRes[i].relList.size() > 0){
			fprintf(stderr, "Release");
			switch(compRes[i].resType){
				case LINK_RES:
					fprintf(stderr, " L(%d-%d)", compRes[i].srcID, compRes[i].dstID);
					break;
				case TRANC_RES:
					fprintf(stderr, " T(%d)", compRes[i].srcID);
					break;
				case INTER_RES:
					fprintf(stderr, " I(%d)", compRes[i].srcID);
					break;
			}
			for(int j = 0; j < (int)compRes[i].relList.size(); j++)
				fprintf(stderr, " F=%d/P=%d(%.2lf)", compRes[i].relList[j].flowID, compRes[i].relList[j].pathID, compRes[i].relList[j].traffic);
			fprintf(stderr, "\n");
		}
		if(compRes[i].reqList.size() > 0){
			fprintf(stderr, "Require");
			switch(compRes[i].resType){
				case LINK_RES:
					fprintf(stderr, " L(%d-%d)", compRes[i].srcID, compRes[i].dstID);
					break;
				case TRANC_RES:
					fprintf(stderr, " T(%d)", compRes[i].srcID);
					break;
				case INTER_RES:
					fprintf(stderr, " I(%d)", compRes[i].srcID);
					break;
			}
			for(int j = 0; j < (int)compRes[i].reqList.size(); j++)
				fprintf(stderr, " F=%d/P=%d(%.2lf)", compRes[i].reqList[j].flowID, compRes[i].reqList[j].pathID, compRes[i].reqList[j].traffic);
			fprintf(stderr, "\n");
		}
	}
	*/
}

// Create competition graph
void Compete::createGraph(const vector<Flow> &allFlow){

	// Variables
	int flowID, pathID;
	CompNode ntmp;
	CompEdge etmp;
	map<int, int>mtmp;
	vector< map<int, int> >flowMap;

	// Create node for each path flow
	for(flowID = 0; flowID < (int)allFlow.size(); flowID++){
		ntmp.flowID = flowID;
		flowMap.push_back(mtmp);
		for(pathID = 0; pathID < (int)allFlow[flowID].flowPath.size(); pathID++){
			ntmp.pathID = pathID;
			flowMap[flowID][pathID] = compNode.size();
			compNode.push_back(ntmp);
		}
	}

	// Create edge if competition exist
	for(int resID = 0; resID < (int)compRes.size(); resID++){
		for(int reqID = 0; reqID < (int)compRes[resID].reqList.size(); reqID++){

			// Cannot get resource at once
			if(compRes[resID].reqList[reqID].traffic > compRes[resID].resCap){

				// No releasing resource: DEADLOCK
				if((int)compRes[resID].relList.size() == 0){
					fprintf(stderr, "Error: no releasing resource (deadlock)\n");
					exit(1);
				}

				// Create edge from requiring to releasing
				flowID = compRes[resID].reqList[reqID].flowID;
				pathID = compRes[resID].reqList[reqID].pathID;
				etmp.resID = resID;
				etmp.dstID = flowMap[flowID][pathID];
				for(int relID = 0; relID < (int)compRes[resID].relList.size(); relID++){
					flowID = compRes[resID].relList[relID].flowID;
					pathID = compRes[resID].relList[relID].pathID;
					compNode[ flowMap[flowID][pathID] ].edge.push_back(etmp);
				}
			}
		}
	}

	// DEBUG: compete graph
	/*
	for(int nodeID = 0; nodeID < (int)compNode.size(); nodeID++){
		if(compNode[nodeID].edge.size() > 0){
			fprintf(stderr, "%d/%d:\n", compNode[nodeID].flowID, compNode[nodeID].pathID);
			for(int edgeID = 0; edgeID < (int)compNode[nodeID].edge.size(); edgeID++){
				int tmpID = compNode[nodeID].edge[edgeID].dstID;
				int resID = compNode[nodeID].edge[edgeID].resID;
				fprintf(stderr, "  %d/%d[", compNode[tmpID].flowID, compNode[tmpID].pathID);
				switch(compRes[resID].resType){
					case LINK_RES:
						fprintf(stderr, "L(%d-%d)", compRes[resID].srcID, compRes[resID].dstID);
						break;
					case TRANC_RES:
						fprintf(stderr, "T(%d)", compRes[resID].srcID);
						break;
					case INTER_RES:
						fprintf(stderr, "I(%d)", compRes[resID].srcID);
						break;
				}
				fprintf(stderr, "]");
			}
			fprintf(stderr, "\n");
		}
	}
	*/
}

// Dfs for checking cycle
void Compete::dfsCycle(int now, int len){

	// Mark as temporary visit
	cycleVis[now] = 1;
	cycleList[len] = now;

	// Traverse all its neighbor
	for(int i = 0; i < (int)compNode[now].edge.size(); i++){

		// Not visit at all
		if(!cycleVis[compNode[now].edge[i].dstID]){
			dfsCycle(compNode[now].edge[i].dstID, len+1);
			if(cycleAns.size() > 0) return;
		}

		// Cycle detected
		else if(cycleVis[compNode[now].edge[i].dstID] == 1){
			int start;
			for(start = 0; start <= len && cycleList[start] != compNode[now].edge[i].dstID; start++);
			for(int i = 0; start+i <= len; i++)
				cycleAns.push_back(cycleList[start+i]);
			return;
		}
	}

	// Mark as final visit
	cycleVis[now] = 2;
}

// Check cycle
bool Compete::checkCycle(void){

	// Initialize
	cycleVis.clear();
	cycleAns.clear();
	cycleList.resize(compNode.size());

	// DFS cycle detection (pick up the first cycle)
	for(int i = 0; i < (int)compNode.size(); i++)
		if(!cycleVis[i] && !cycleAns.size())
			dfsCycle(i, 0);

	// Return checking result
	return cycleAns.size() > 0;
}

// Change current plan to new plan
void Compete::changePlan(const vector<Link>& initLink, const vector<Flow>& allFlow, vector<Flow>& newFlow1, vector<Flow>& newFlow2, int k){

	// Variable
	int totalCnt;
	int srcID, dstID;
	int flowID, pathID, linkID;
	int nowID, nxtID;
	bool done, interOK;
	double traffic;
	BFSNode bfsNow, bfsNxt;
	vector<int>etmp;
	vector< vector<int> >edg;
	vector<CompRes>copyRes;
	map<int, int>prev;
	map<int, bool>vis;
	queue<BFSNode>que;
	Flow ftmp;
	FlowPath ptmp;
	Link ltmp;

	// Copy the original plan
	for(int flowID = 0; flowID < (int)allFlow.size(); flowID++)
		newFlow1.push_back(allFlow[flowID]);

	// Initialize edge structure
	totalCnt = 5*k*k/4;
	for(int i = 0; i < totalCnt; i++)
		edg.push_back(etmp);

	// Copy wireless links only
	for(int i = 0; i < (int)initLink.size(); i++)
		if(initLink[i].isWireless)
			edg[initLink[i].sourceID].push_back(initLink[i].destinationID);

	// Final path of 2, 4, 6, 8, ... flow occupy the resource
	for(int i = 0; i < (int)cycleAns.size(); i++){
		if(i%2){
			flowID = compNode[cycleAns[i]].flowID;
			pathID = compNode[cycleAns[i]].pathID;
			traffic = allFlow[flowID].flowPath[pathID].traffic;
			for(int j = 0; j < (int)allFlow[flowID].flowPath[pathID].link[1].size(); j++){
				srcID = allFlow[flowID].flowPath[pathID].link[1][j].sourceID;
				dstID = allFlow[flowID].flowPath[pathID].link[1][j].destinationID;
				linkID = linkMap[srcID][dstID];

				// All links
				compRes[linkID].resCap -= traffic;

				// Wireless links
				if(compRes[linkID].isWireless){

					// Transceiver
					compRes[ trancMap[srcID] ].resCap -= traffic;
					compRes[ trancMap[dstID] ].resCap -= traffic;

					// Interference
					for(int z = 0; z < (int)compRes[linkID].iList.size(); z++)
						compRes[ compRes[linkID].iList[z] ].resCap -= traffic;
				}
			}
		}
	}

	// Find out alternative path for 1, 3, 5, 7, ... path
	for(int i = 0; i < (int)cycleAns.size(); i++){
		if(!(i%2)){
			flowID = compNode[cycleAns[i]].flowID;
			pathID = compNode[cycleAns[i]].pathID;
			srcID = allFlow[flowID].ingressID;
			dstID = allFlow[flowID].flowPath[pathID].dstID[1];
			traffic = allFlow[flowID].flowPath[pathID].traffic;

			// Copy resource usage
			bfsNow.interCap.clear();
			for(int j = 0; j < (int)compRes.size(); j++){
				copyRes.push_back(compRes[j]);
				if(copyRes[j].resType == INTER_RES)
					bfsNow.interCap[ copyRes[j].srcID ] = LINK_CAPACITY-copyRes[j].resCap;
			}

			// BFS
			done = false;
			bfsNow.switchID = srcID;
			que.push(bfsNow);
			vis[srcID] = true;
			prev[srcID] = srcID;
			while(!que.empty() && !done){
				bfsNow = que.front();
				nowID = bfsNow.switchID;
				que.pop();
				if(copyRes[ trancMap[nowID] ].resCap < traffic) continue;

				// Search neighbor
				for(int j = 0; j < (int)edg[nowID].size(); j++){
					nxtID = edg[nowID][j];
					bfsNxt = bfsNow;
					bfsNxt.switchID = nxtID;
					if(copyRes[ trancMap[nxtID] ].resCap < traffic) continue;

					// Un-visited
					if(!vis[nxtID]){

						// Check interference
						interOK = true;
						linkID = linkMap[nowID][nxtID];
						for(int k = 0; k < (int)copyRes[linkID].iList.size(); k++){
							if(bfsNow.interCap[ copyRes[ copyRes[linkID].iList[k] ].srcID ] + traffic > LINK_CAPACITY){
								interOK = false;
								break;
							}
							else{
								// Update
								bfsNxt.interCap[ copyRes[ copyRes[linkID].iList[k] ].srcID ] += traffic;
							}
						}

						// Feasible
						if(interOK){
							que.push(bfsNxt);
							vis[nxtID] = true;
							prev[nxtID] = nowID;

							// Destination
							if(nxtID == dstID){
								done = true;
								break;
							}
						}
					}
				}
			}

			// If found
			if(done){

				// DEBUG: print out path
				nowID = dstID;
				while(nowID != srcID){
					fprintf(stderr, "%d <-- ", nowID);
					nowID = prev[nowID];
				}
				fprintf(stderr, "%d\n", srcID);

				// Modify flow transition info in newFlow1
				// TODO: THIS IS ONLY HOP INFO, NOT THE FORMAY AS allFlow
				newFlow1[flowID].flowPath[pathID].link[1].clear();
				nowID = dstID;
				while(nowID != srcID){
					ltmp.sourceID = prev[nowID];
					ltmp.destinationID = nowID;
					newFlow1[flowID].flowPath[pathID].link[1].push_back(ltmp);
					nowID = prev[nowID];
				}
				// TODO: sort hop

				// Add flow transition info in newFlow2
				// TODO: THE SAME FLOW OF DIFF PATHS SHOULD BE COMBINED INTO ONE
				ftmp.flowID = newFlow2.size();
				ftmp.flowTag = flowID;
				ftmp.ingressID = srcID;
				ptmp.link[0] = newFlow1[flowID].flowPath[pathID].link[1];
				ptmp.link[1].clear();
				for(int j = 0; j < (int)allFlow[flowID].flowPath[pathID].link[1].size(); j++){
					ptmp.link[1].push_back(allFlow[flowID].flowPath[pathID].link[1][j]);
				}
				ftmp.flowPath.push_back(ptmp);
				newFlow2.push_back(ftmp);
				ptmp.link[0].clear();
				ptmp.link[1].clear();
				ftmp.flowPath.clear();

				// Update back the original resource usage
				fprintf(stderr, "TODO: update back the original resource usage\n");
			}

			// Clear
			vis.clear();
			prev.clear();
			copyRes.clear();
			while(!que.empty()) que.pop();
		}
	}
	// TODO: and change the new paths
	
}

// Destructure
Compete::~Compete(){
	
	// Clear all
	this->linkMap.clear();
	this->trancMap.clear();
	this->interMap.clear();
	this->compRes.clear();
	this->compNode.clear();
}

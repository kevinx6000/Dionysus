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

	// Copy final resource usage
	lastRes = compRes;

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

	// Update intial resource consumption
	for(int i = 0; i < (int)allFlow.size(); i++)
		for(int j = 0; j < (int)allFlow[i].flowPath.size(); j++)
			occupyRes(allFlow[i].flowPath[j].link[0], allFlow[i].flowPath[j].traffic, compRes);

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
					compNode[ etmp.dstID ].prev.push_back(flowMap[flowID][pathID]);
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

// Backtracking for vertex cover
void Compete::backtrack(int now, int cnt, int n){

	// Variable
	int mark;
	bool canbe;

	// End
	if(now == n){
		
		// Update if better
		if(cnt < mvcSize){
			mvcSize = cnt;
			for(int i = 0; i < n; i++)
				mvcList[i] = visMark[i];
		}
		return;
	}

	// Prune: never smaller than current
	if(cnt >= mvcSize) return;

	// Visit
	visMark[now] = VISITED;

	// Check if can be WHITE
	canbe = true;
	for(int i = 0; i < (int)compNode[now].edge.size(); i++){
		mark = visMark[ compNode[now].edge[i].dstID ];
		if(mark == WHITE){
			canbe = false;
			break;
		}
	}
	for(int i = 0; i < (int)compNode[now].prev.size(); i++){
		mark = visMark[ compNode[now].prev[i] ];
		if(mark == WHITE){
			canbe = false;
			break;
		}
	}

	// Can be WHITE
	if(canbe){
		visMark[now] = WHITE;
		backtrack(now+1, cnt, n);
	}

	// Always can be BLACK
	visMark[now] = BLACK;
	backtrack(now+1, cnt+1, n);

	// Leave
	visMark[now] = NOT_VISITED;
}

// Check if temporary resource is needed
bool Compete::needTemp(void){

	// Initialize
	mvcSize = compNode.size();
	mvcList.clear();
	for(int i = 0; i < mvcSize; i++)
		mvcList.push_back(NOT_VISITED);
	visMark.clear();

	// Minimum Vertex Cover
	backtrack(0, 0, compNode.size());

	// Return checking result
	return mvcSize > 0;
}

// Change current plan to new plan
void Compete::changePlan(const vector<Link>& initLink, const vector<Flow>& allFlow, vector<Flow>& newFlow1, vector<Flow>& newFlow2, int k){

	// Variable
	int totalCnt;
	int srcID, dstID;
	int flowID, pathID;
	double traffic;
	vector<int>etmp;
	vector< vector<int> >edg;
	vector<Link>newPath;
	ResDiff resDiff, resDiff2;

	// Copy the original plan
	newFlow1 = allFlow;
	newFlow2 = allFlow;

	// Initialize edge structure
	totalCnt = 5*k*k/4;
	for(int i = 0; i < totalCnt; i++)
		edg.push_back(etmp);

	// Copy wireless links only
	for(int i = 0; i < (int)initLink.size(); i++)
		if(initLink[i].isWireless)
			edg[initLink[i].sourceID].push_back(initLink[i].destinationID);

	// Occupy resource of stage1
	// Resource: all initial state (already occupied) and WHITE final state
	for(int i = 0; i < (int)compNode.size(); i++){
		flowID = compNode[i].flowID;
		pathID = compNode[i].pathID;
		if(mvcList[i] == WHITE){

			// Update the requiring part
			resDiffCheck(newFlow1[flowID].ingressID, newFlow1[flowID].flowPath[pathID], resDiff);
			for(int j = 0; j < (int)resDiff.link.size(); j++)
				compRes[ resDiff.link[j].ID ].resCap -= resDiff.link[j].reqTraffic;
			for(int j = 0; j < (int)resDiff.tranc.size(); j++)
				compRes[ resDiff.tranc[j].ID ].resCap -= resDiff.tranc[j].reqTraffic;
			for(int j = 0; j < (int)resDiff.inter.size(); j++)
				compRes[ resDiff.inter[j].ID ].resCap -= resDiff.inter[j].reqTraffic;
		}
	}

	// Occupy resource of stage2
	// Resource: all final state
	for(int i = 0; i < (int)compNode.size(); i++){
		flowID = compNode[i].flowID;
		pathID = compNode[i].pathID;
		occupyRes(newFlow1[flowID].flowPath[pathID].link[1], newFlow1[flowID].flowPath[pathID].traffic, lastRes);
	}

	// For all nodes in competitive graph
	for(int i = 0; i < (int)compNode.size(); i++){
		flowID = compNode[i].flowID;
		pathID = compNode[i].pathID;

		// WHITE nodes in MVC: no plan change needed
		if(mvcList[i] == WHITE){

			// Copy: I -> F, F -> F
			newFlow2[flowID].flowPath[pathID].link[0] = newFlow2[flowID].flowPath[pathID].link[1];
		}

		// Black nodes in MVC: find out alternative path
		else if(mvcList[i] == BLACK){
			srcID = allFlow[flowID].ingressID;
			dstID = allFlow[flowID].flowPath[pathID].dstID[1];
			traffic = allFlow[flowID].flowPath[pathID].traffic;

			// It's impossible to find out a path using wireless links with traffic*2 > LINK_CAPACITY

			// Try to find out an alternative path
			if(traffic * 2 <= LINK_CAPACITY &&
				alterPath(edg, compRes, lastRes, srcID, dstID, traffic, newFlow1[flowID].flowPath[pathID], newPath, resDiff, resDiff2)){

				// Set new path as  final  state of newFlow1: I -> F'
				newFlow1[flowID].flowPath[pathID].link[1] = newPath;

				// Set new path as initial state of newFlow2: F' -> F
				newFlow2[flowID].flowPath[pathID].link[0] = newPath;

				// Update the requiring part (STAGE1)
				for(int j = 0; j < (int)resDiff.link.size(); j++)
					compRes[ resDiff.link[j].ID ].resCap -= resDiff.link[j].reqTraffic;
				for(int j = 0; j < (int)resDiff.tranc.size(); j++)
					compRes[ resDiff.tranc[j].ID ].resCap -= resDiff.tranc[j].reqTraffic;
				for(int j = 0; j < (int)resDiff.inter.size(); j++)
					compRes[ resDiff.inter[j].ID ].resCap -= resDiff.inter[j].reqTraffic;

				// Update the requiring part (STAGE2)
				for(int j = 0; j < (int)resDiff2.link.size(); j++)
					lastRes[ resDiff2.link[j].ID ].resCap -= resDiff2.link[j].reqTraffic;
				for(int j = 0; j < (int)resDiff2.tranc.size(); j++)
					lastRes[ resDiff2.tranc[j].ID ].resCap -= resDiff2.tranc[j].reqTraffic;
				for(int j = 0; j < (int)resDiff2.inter.size(); j++)
					lastRes[ resDiff2.inter[j].ID ].resCap -= resDiff2.inter[j].reqTraffic;

				// DEBUG message
				fprintf(stderr, "[Info] Alternative path found\n");
			}

			// Not found, preserve the original one
			else{

				// Copy: I -> F, F -> F
				newFlow2[flowID].flowPath[pathID].link[0] = newFlow2[flowID].flowPath[pathID].link[1];

				// Update the requiring part
				resDiffCheck(newFlow1[flowID].ingressID, newFlow1[flowID].flowPath[pathID], resDiff);
				for(int j = 0; j < (int)resDiff.link.size(); j++)
					compRes[ resDiff.link[j].ID ].resCap -= resDiff.link[j].reqTraffic;
				for(int j = 0; j < (int)resDiff.tranc.size(); j++)
					compRes[ resDiff.tranc[j].ID ].resCap -= resDiff.tranc[j].reqTraffic;
				for(int j = 0; j < (int)resDiff.inter.size(); j++)
					compRes[ resDiff.inter[j].ID ].resCap -= resDiff.inter[j].reqTraffic;

				// DEBUG message
				fprintf(stderr, "[Info] Alternative path not found, preserve original path\n");
			}
		}

		// Exception
		else{
			fprintf(stderr, "[Error] Color except white or black exists.\n");
			exit(1);
		}
	}
}

// Occupy 
void Compete::occupyRes(const vector<Link>& hopList, double traffic, vector<CompRes>& curRes){

	// Update along the path
	for(int hop = 0; hop < (int)hopList.size(); hop++){
		int srcID = hopList[hop].sourceID;
		int dstID = hopList[hop].destinationID;
		int linkID = linkMap[srcID][dstID];

		// All links
		curRes[linkID].resCap -= traffic;

		// Wireless links
		if(curRes[linkID].isWireless){

			// Transceiver
			curRes[ trancMap[srcID] ].resCap -= traffic;
			curRes[ trancMap[dstID] ].resCap -= traffic;

			// Interference
			for(int z = 0; z < (int)curRes[linkID].iList.size(); z++)
				curRes[ curRes[linkID].iList[z] ].resCap -= traffic;
		}
	}
}

// Alternative path searching
bool Compete::alterPath(const vector< vector<int> >& edg, const vector<CompRes>& stage1, const vector<CompRes>& stage2,
		int srcID, int dstID, double traffic, const FlowPath& orgPlan, vector<Link>& newPath, ResDiff& diff1, ResDiff& diff2){

	// Variable
	int nowID, nxtID, linkID, tmpID;
	bool done, interOK;
	Link ltmp;
	ResDiffNode rtmp;
	map<int, int>prev;
	map<int, bool>vis;
	map<int, bool>mtmp;
	queue<BFSNode>que;
	BFSNode bfsNow, bfsNxt, bfsFinal;
	vector< map<int, bool> >used1;
	vector< map<int, bool> >used2;
	map<int, int>interCnt1;
	map<int, int>interCnt2;
	map<int, int>::iterator mapItr;

	// Initialize all links as not used
	mtmp.clear();
	for(int i = 0; i < (int)linkMap.size(); i++){
		used1.push_back(mtmp);
		used2.push_back(mtmp);
	}

	// Clear resource difference
	diff1.link.clear();
	diff1.tranc.clear();
	diff1.inter.clear();
	diff2.link.clear();
	diff2.tranc.clear();
	diff2.inter.clear();

	// Record interference count for original plan
	interCnt1.clear();
	interCnt2.clear();
	for(int state = 0; state < 2; state++){
		for(int i = 0; i < (int)orgPlan.link[state].size(); i++){
			nowID = orgPlan.link[state][i].sourceID;
			nxtID = orgPlan.link[state][i].destinationID;
			linkID = linkMap[nowID][nxtID];

			// Stage1
			if(state == 1){
				used1[nowID][nxtID] = true;
				for(int j = 0; j < (int)stage1[linkID].iList.size(); j++)
					interCnt1[ stage1[ stage1[linkID].iList[j] ].srcID ]++;
			}

			// State2
			else{
				used2[nowID][nxtID] = true;
				for(int j = 0; j < (int)stage2[linkID].iList.size(); j++)
					interCnt2[ stage2[ stage2[linkID].iList[j] ].srcID ]++;
			}
		}
	}

	// Clear BFS node records
	bfsNow.stage1.interCnt.clear();
	bfsNow.stage1.linkCap.clear();
	bfsNow.stage1.trancCap.clear();
	bfsNow.stage1.interCap.clear();
	bfsNow.stage2.interCnt.clear();
	bfsNow.stage2.linkCap.clear();
	bfsNow.stage2.trancCap.clear();
	bfsNow.stage2.interCap.clear();

	// Copy resource usage for stage1
	for(int i = 0; i < (int)stage1.size(); i++){
		if(stage1[i].resType == LINK_RES)
			bfsNow.stage1.linkCap[i] = stage1[i].resCap;
		if(stage1[i].resType == TRANC_RES)
			bfsNow.stage1.trancCap[ stage1[i].srcID ] = stage1[i].resCap;
		if(stage1[i].resType == INTER_RES)
			bfsNow.stage1.interCap[ stage1[i].srcID ] = stage1[i].resCap;
	}

	// Copy resource usage for stage2
	for(int i = 0; i < (int)stage2.size(); i++){
		if(stage2[i].resType == LINK_RES)
			bfsNow.stage2.linkCap[i] = stage2[i].resCap;
		if(stage2[i].resType == TRANC_RES)
			bfsNow.stage2.trancCap[ stage2[i].srcID ] = stage2[i].resCap;
		if(stage2[i].resType == INTER_RES)
			bfsNow.stage2.interCap[ stage2[i].srcID ] = stage2[i].resCap;
	}

fprintf(stderr, "<-------------- BFS start ------------->\n");
clock_t begTime = clock();
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

		// Search neighbor
		for(int i = 0; i < (int)edg[nowID].size(); i++){
			nxtID = edg[nowID][i];
			if(!vis[nxtID]){
				bfsNxt = bfsNow;
				bfsNxt.switchID = nxtID;

				// Stage1: not in original plan -> new requiring resource
				if(!used1[nowID][nxtID]){

					// Link capacity
					if(bfsNxt.stage1.linkCap[ linkMap[nowID][nxtID] ] < traffic) continue;
					bfsNxt.stage1.linkCap[ linkMap[nowID][nxtID] ] -= traffic;

					// Wireless: further check for transceiver/interference
					if(stage1[ linkMap[nowID][nxtID] ].isWireless){

						// Transceiver
						if(bfsNxt.stage1.trancCap[nowID] < traffic) continue;
						bfsNxt.stage1.trancCap[nowID] -= traffic;
						if(bfsNxt.stage1.trancCap[nxtID] < traffic) continue;
						bfsNxt.stage1.trancCap[nxtID] -= traffic;

						// Interference
						interOK = true;
						linkID = linkMap[nowID][nxtID];
						for(int j = 0; j < (int)stage1[linkID].iList.size(); j++){

							// Current interference count
							tmpID = stage1[ stage1[linkID].iList[j] ].srcID;
							bfsNxt.stage1.interCnt[tmpID]++;

							// Not exceed existing interference count: no need to used
							if(bfsNxt.stage1.interCnt[tmpID] <= interCnt1[tmpID]) continue;

							// Not enough capacity
							if(bfsNxt.stage1.interCap[tmpID] < traffic){
								interOK = false;
								break;
							}
							bfsNxt.stage1.interCap[tmpID] -= traffic;
						}
						if(!interOK) continue;
					}
				}

				// Stage2: not in original plan -> new requiring resource
				if(!used2[nowID][nxtID]){

					// Link capacity
					if(bfsNxt.stage2.linkCap[ linkMap[nowID][nxtID] ] < traffic) continue;
					bfsNxt.stage2.linkCap[ linkMap[nowID][nxtID] ] -= traffic;

					// Wireless: further check for transceiver/interference
					if(stage2[ linkMap[nowID][nxtID] ].isWireless){

						// Transceiver
						if(bfsNxt.stage2.trancCap[nowID] < traffic) continue;
						bfsNxt.stage2.trancCap[nowID] -= traffic;
						if(bfsNxt.stage2.trancCap[nxtID] < traffic) continue;
						bfsNxt.stage2.trancCap[nxtID] -= traffic;

						// Interference
						interOK = true;
						linkID = linkMap[nowID][nxtID];
						for(int j = 0; j < (int)stage2[linkID].iList.size(); j++){

							// Current interference count
							tmpID = stage2[ stage2[linkID].iList[j] ].srcID;
							bfsNxt.stage2.interCnt[tmpID]++;

							// Not exceed existing interference count: no need to used
							if(bfsNxt.stage2.interCnt[tmpID] <= interCnt2[tmpID]) continue;

							// Not enough capacity
							if(bfsNxt.stage2.interCap[tmpID] < traffic){
								interOK = false;
								break;
							}
							bfsNxt.stage2.interCap[tmpID] -= traffic;
						}
						if(!interOK) continue;
					}
				}

				// Feasible
				que.push(bfsNxt);
				vis[nxtID] = true;
				prev[nxtID] = nowID;

				// Reach destination
				if(nxtID == dstID){
					done = true;
					bfsFinal = bfsNxt;
					break;
				}
			}
		}
	}
clock_t endTime = clock();
fprintf(stderr, "[Info] time = %.2lf msec\n", (endTime - begTime) / 1000.0);
fprintf(stderr, "<-------------- BFS End ------------->\n");

	// Path found
	if(done){

		// DEBUG: print out path
		nowID = dstID;
		while(nowID != srcID){
			fprintf(stderr, "%d <-- ", nowID);
			nowID = prev[nowID];
		}
		fprintf(stderr, "%d\n", srcID);

		// Retrieve new path
		newPath.clear();
		nowID = dstID;
		while(nowID != srcID){
			ltmp.sourceID = prev[nowID];
			ltmp.destinationID = nowID;
			newPath.push_back(ltmp);
			nowID = prev[nowID];
		}

		// Record traffic difference for stage1
		rtmp.relTraffic = 0;
		for(int i = 0; i < (int)stage1.size(); i++){
			if(stage1[i].resType == LINK_RES){
				if(bfsFinal.stage1.linkCap[i] < stage1[i].resCap){
					rtmp.ID = i;
					rtmp.reqTraffic = stage1[i].resCap - bfsFinal.stage1.linkCap[i];
					diff1.link.push_back(rtmp);
				}
			}
			if(stage1[i].resType == TRANC_RES){
				if(bfsFinal.stage1.trancCap[ stage1[i].srcID ] < stage1[i].resCap){
					rtmp.ID = i;
					rtmp.reqTraffic = stage1[i].resCap - bfsFinal.stage1.trancCap[stage1[i].srcID];
					diff1.tranc.push_back(rtmp);
				}
			}
			if(stage1[i].resType == INTER_RES){
				if(bfsFinal.stage1.interCap[ stage1[i].srcID ] < stage1[i].resCap){
					rtmp.ID = i;
					rtmp.reqTraffic = stage1[i].resCap - bfsFinal.stage1.interCap[stage1[i].srcID];
					diff1.inter.push_back(rtmp);
				}
			}
		}

		// Record traffic difference for stage1
		rtmp.relTraffic = 0;
		for(int i = 0; i < (int)stage2.size(); i++){
			if(stage2[i].resType == LINK_RES){
				if(bfsFinal.stage2.linkCap[i] < stage2[i].resCap){
					rtmp.ID = i;
					rtmp.reqTraffic = stage2[i].resCap - bfsFinal.stage2.linkCap[i];
					diff2.link.push_back(rtmp);
				}
			}
			if(stage2[i].resType == TRANC_RES){
				if(bfsFinal.stage2.trancCap[ stage2[i].srcID ] < stage2[i].resCap){
					rtmp.ID = i;
					rtmp.reqTraffic = stage2[i].resCap - bfsFinal.stage2.trancCap[stage2[i].srcID];
					diff2.tranc.push_back(rtmp);
				}
			}
			if(stage2[i].resType == INTER_RES){
				if(bfsFinal.stage2.interCap[ stage2[i].srcID ] < stage2[i].resCap){
					rtmp.ID = i;
					rtmp.reqTraffic = stage2[i].resCap - bfsFinal.stage2.interCap[stage2[i].srcID];
					diff2.inter.push_back(rtmp);
				}
			}
		}
	}

	// Clear (not necessary)
	vis.clear();
	prev.clear();
	used1.clear();
	used2.clear();
	while(!que.empty()) que.pop();

	// Return true if found
	return done;
}

// Resource difference checker
void Compete::resDiffCheck(int ingressID, FlowPath& flowPath, ResDiff& resDiff){

	// Variables
	int ptr1, ptr2, ingPtr1, ingPtr2;
	int sID1, sID2, dID1, dID2, linkID, interID;
	int siz1, siz2;
	int relCnt, reqCnt;
	double traffic;
	set<int>trancSwitch;
	set<int>interSwitch;
	set<int>::iterator setItr;
	ResDiffNode rtmp;

	// Sort initial and final hops
	sort(flowPath.link[0].begin(), flowPath.link[0].end(), cmpHop);
	sort(flowPath.link[1].begin(), flowPath.link[1].end(), cmpHop);

	// Initialize transceiver and interference resource to zero
	trancSwitch.clear();
	interSwitch.clear();
	for(int state = 0; state < 2; state++){
		for(int hop = 0; hop < (int)flowPath.link[state].size(); hop++){
			sID1 = flowPath.link[state][hop].sourceID;
			dID1 = flowPath.link[state][hop].destinationID;
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

	// Initialize resource difference variable
	resDiff.link.clear();
	resDiff.tranc.clear();
	resDiff.inter.clear();
	
	// Check the difference between initial and final state
	ptr1 = ptr2 = 0;
	siz1 = flowPath.link[0].size();
	siz2 = flowPath.link[1].size();
	traffic = flowPath.traffic;
	while(ptr1 < siz1 && ptr2 < siz2){
		sID1 = flowPath.link[0][ptr1].sourceID;
		dID1 = flowPath.link[0][ptr1].destinationID;
		sID2 = flowPath.link[1][ptr2].sourceID;
		dID2 = flowPath.link[1][ptr2].destinationID;
		if(sID1 == ingressID){
			ingPtr1 = ptr1++;
			continue;
		}
		if(sID2 == ingressID){
			ingPtr2 = ptr2++;
			continue;
		}

		// Same switch
		if(sID1 == sID2){
			if(dID1 != dID2){

				// Release link: sID1-dID1
				rtmp.ID = linkMap[sID1][dID1];
				rtmp.reqTraffic = 0;
				rtmp.relTraffic = traffic;
				resDiff.link.push_back(rtmp);

				// Require link: sID2-dID2
				rtmp.ID = linkMap[sID2][dID2];
				rtmp.reqTraffic = traffic;
				rtmp.relTraffic = 0;
				resDiff.link.push_back(rtmp);
			}
			ptr1++;
			ptr2++;
		}

		// Add switch sID2
		else if(sID1 > sID2){

			// Require link: sID2-dID2
			rtmp.ID = linkMap[sID2][dID2];
			rtmp.reqTraffic = traffic;
			rtmp.relTraffic = 0;
			resDiff.link.push_back(rtmp);
			ptr2++;
		}

		// Del switch sID1
		else{

			// Release link: sID1-dID1
			rtmp.ID = linkMap[sID1][dID1];
			rtmp.reqTraffic = 0;
			rtmp.relTraffic = traffic;
			resDiff.link.push_back(rtmp);
			ptr1++;
		}
	}

	// Remaining old switches
	while(ptr1 < siz1){
		sID1 = flowPath.link[0][ptr1].sourceID;
		dID1 = flowPath.link[0][ptr1].destinationID;
		if(sID1 == ingressID){
			ingPtr1 = ptr1++;
			continue;
		}
		// Release link: sID1-dID1
		rtmp.ID = linkMap[sID1][dID1];
		rtmp.reqTraffic = 0;
		rtmp.relTraffic = traffic;
		resDiff.link.push_back(rtmp);
		ptr1++;
	}

	// Remaining new switches
	while(ptr2 < siz2){
		sID2 = flowPath.link[1][ptr2].sourceID;
		dID2 = flowPath.link[1][ptr2].destinationID;
		if(sID2 == ingressID){
			ingPtr2 = ptr2++;
			continue;
		}
		// Require link: sID2-dID2
		rtmp.ID = linkMap[sID2][dID2];
		rtmp.reqTraffic = traffic;
		rtmp.relTraffic = 0;
		resDiff.link.push_back(rtmp);
		ptr2++;
	}

	// Ingress switch
	sID1 = ingressID;
	dID1 = flowPath.link[0][ingPtr1].destinationID;
	dID2 = flowPath.link[1][ingPtr2].destinationID;
	if(dID1 != dID2){

		// Release link: sID1-dID1
		rtmp.ID = linkMap[sID1][dID1];
		rtmp.reqTraffic = 0;
		rtmp.relTraffic = traffic;
		resDiff.link.push_back(rtmp);

		// Require link: sID1-dID2
		rtmp.ID = linkMap[sID1][dID2];
		rtmp.reqTraffic = traffic;
		rtmp.relTraffic = 0;
		resDiff.link.push_back(rtmp);
	}

	// Release/require count for transceiver and interference resource
	for(int state = 0; state < 2; state++){
		for(int hop = 0; hop < (int)flowPath.link[state].size(); hop++){
			sID1 = flowPath.link[state][hop].sourceID;
			dID1 = flowPath.link[state][hop].destinationID;
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

			// Require tranc: (reqCnt - relCnt) * traffic
			rtmp.ID = *setItr;
			rtmp.reqTraffic = (reqCnt - relCnt) * traffic;
			rtmp.relTraffic = 0;
			resDiff.tranc.push_back(rtmp);
		}
		else if(relCnt > reqCnt){

			// Release tranc: (relCnt - reqCnt) * traffic
			rtmp.ID = *setItr;
			rtmp.reqTraffic = 0;
			rtmp.relTraffic = (relCnt - reqCnt) * traffic;
			resDiff.tranc.push_back(rtmp);
		}
	}
	for(setItr = interSwitch.begin(); setItr != interSwitch.end(); setItr++){
		relCnt = compRes[ *setItr ].relCnt;
		reqCnt = compRes[ *setItr ].reqCnt;
		if(relCnt < reqCnt){

			// Require tranc: (reqCnt - relCnt) * traffic
			rtmp.ID = *setItr;
			rtmp.reqTraffic = (reqCnt - relCnt) * traffic;
			rtmp.relTraffic = 0;
			resDiff.inter.push_back(rtmp);
		}
		else if(relCnt > reqCnt){

			// Release tranc: (relCnt - reqCnt) * traffic
			rtmp.ID = *setItr;
			rtmp.reqTraffic = 0;
			rtmp.relTraffic = (relCnt - reqCnt) * traffic;
			resDiff.inter.push_back(rtmp);
		}
	}

	// DEBUG
	fprintf(stderr, "[Info] Resource Diff (link)\n");
	for(int i = 0; i < (int)resDiff.link.size(); i++)
		fprintf(stderr, "[Info]\t%d-%d: req=%.2lf, rel=%.2lf\n", compRes[resDiff.link[i].ID].srcID, compRes[resDiff.link[i].ID].dstID, resDiff.link[i].reqTraffic, resDiff.link[i].relTraffic);
	fprintf(stderr, "[Info] Resource Diff (tranc)\n");
	for(int i = 0; i < (int)resDiff.tranc.size(); i++)
		fprintf(stderr, "[Info]\t%d: req=%.2lf, rel=%.2lf\n", compRes[resDiff.tranc[i].ID].srcID, resDiff.tranc[i].reqTraffic, resDiff.tranc[i].relTraffic);
	fprintf(stderr, "[Info] Resource Diff (inter)\n");
	for(int i = 0; i < (int)resDiff.inter.size(); i++)
		fprintf(stderr, "[Info]\t%d: req=%.2lf, rel=%.2lf\n", compRes[resDiff.inter[i].ID].srcID, resDiff.inter[i].reqTraffic, resDiff.inter[i].relTraffic);
}

// Comparison function for sorting hops
bool Compete::cmpHop(Link A, Link B){
	return A.sourceID < B.sourceID;
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

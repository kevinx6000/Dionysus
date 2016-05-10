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
		this->linkMap[ ctmp.srcID ][ ctmp.dstID ] = this->linkRes.size();
		this->linkRes.push_back(ctmp);
	}

	// Copy transceiver node
	for(int i = 0; i < (int)initTranc.size(); i++){
		ctmp.srcID = initTranc[i].switchID;
		ctmp.resCap = LINK_CAPACITY;
		this->trancMap[ ctmp.srcID ] = this->trancRes.size();
		this->trancRes.push_back(ctmp);
	}

	// Copy interference node
	for(int i = 0; i < (int)initInter.size(); i++){
		ctmp.srcID = initInter[i].switchID;
		ctmp.resCap = LINK_CAPACITY;
		this->interMap[ ctmp.srcID ] = this->interRes.size();
		this->interRes.push_back(ctmp);
	}

	// Create interference list in terms of the indexing here
	for(int i = 0; i < (int)initLinks.size(); i++){
		srcID = initLinks[i].sourceID;
		dstID = initLinks[i].destinationID;
		linkID = linkMap[srcID][dstID];
		for(int j = 0; j < (int)initLinks[i].iList.size(); j++)
			this->linkRes[linkID].iList.push_back( interMap[ initLinks[i].iList[j] ] );
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
				linkRes[linkID].resCap -= traffic;

				// Wireless links
				if(linkRes[linkID].isWireless){

					// Transceiver
					trancRes[ trancMap[srcID] ].resCap -= traffic;
					trancRes[ trancMap[dstID] ].resCap -= traffic;

					// Interference
					for(int z = 0; z < (int)linkRes[linkID].iList.size(); z++)
						interRes[ linkRes[linkID].iList[z] ].resCap -= traffic;
				}
			}
		}
	}
}

// Destructure
Compete::~Compete(){
	
	// Clear all
	this->linkMap.clear();
	this->trancMap.clear();
	this->interMap.clear();
	this->linkRes.clear();
	this->trancRes.clear();
	this->interRes.clear();
}

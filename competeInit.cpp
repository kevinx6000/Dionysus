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
void Compete::initialize(const vector<Link>& initLinks, const vector<TrancNode>& initTranc, const vector<InterNode>& initInter){

	// Variables
	Link ltmp;
	TrancNode ttmp;
	InterNode itmp;

	// Copy links
	for(int i = 0; i < (int)initLinks.size(); i++){
		ltmp.sourceID = initLinks[i].sourceID;
		ltmp.destinationID = initLinks[i].destinationID;
		ltmp.linkCapacity = initLinks[i].linkCapacity;
		this->links.push_back(ltmp);
	}

	// Copy transceiver node
	for(int i = 0; i < (int)initTranc.size(); i++){
		ttmp.switchID = initTranc[i].switchID;
		ttmp.nodeCapacity = initTranc[i].nodeCapacity;
		this->tranc.push_back(ttmp);
	}

	// Copy interference node
	for(int i = 0; i < (int)initInter.size(); i++){
		itmp.switchID = initInter[i].switchID;
		itmp.nodeCapacity = initInter[i].nodeCapacity;
		this->inter.push_back(itmp);
	}

	// Set as already initialized
	isInitLink = true;
	isInitTranc = true;
	isInitInter = true;
}

// Constructor
Compete::Compete(const vector<Link>& initLinks, const vector<TrancNode>& initTranc, const vector<InterNode>& initInter){
	this->initialize(initLinks, initTranc, initInter);
}

// Update resource
void Compete::updateResource(const vector<Flow>& allFlow){

	// Variables
	int srcID;
	int dstID;

	// Update intial resource consumption
	for(int i = 0; i < (int)allFlow.size(); i++){
		for(int j = 0; j < (int)allFlow[i].flowPath.size(); j++){
			// Link node
			for(int k = 0; k < (int)allFlow[i].flowPath[j].link[0].size(); k++){
				srcID = allFlow[i].flowPath[j].link[0][k].sourceID;
				dstID = allFlow[i].flowPath[j].link[0][k].destinationID;
			}
		}
	}
	// TODO: 1. 修改resource structure 2. 更新resource nodes
}

// Destructure
Compete::~Compete(){
	
	// Clear all
	this->links.clear();
	this->tranc.clear();
	this->inter.clear();
}

// Header
#include "header.h"
#include "structure.h"
#include "compete.h"

// Empty constructure
Compete::Compete(){
	isInitLink = false;
	isInitTranc = false;
	isInitInter = false;
}

// Initialize constructure
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
	// Call constructor
	this->initialize(initLinks, initTranc, initInter);
}

// Destructure
Compete::~Compete(){
	
	// Clear all
	this->links.clear();
	this->tranc.clear();
	this->inter.clear();
}

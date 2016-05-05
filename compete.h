#ifndef COMPETE_H
#define COMPETE_H

// Header
#include "header.h"
#include "structure.h"

// Compete Graph
class Compete{
	public:
		Compete();
		Compete(const vector<Link>&, const vector<TrancNode>&, const vector<InterNode>&);
		void initialize(const vector<Link>&, const vector<TrancNode>&, const vector<InterNode>&);
		~Compete();
	private:
		bool isInitLink;		// Record if link resource is initialized
		bool isInitTranc;		// Record if transceiver resource is initialized
		bool isInitInter;		// Record if interference resource is initialized
		vector<Link>links;		// Link resource
		vector<TrancNode>tranc;	// Transceiver resource
		vector<InterNode>inter;	// Interference resource
};

#endif

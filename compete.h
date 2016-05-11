#ifndef COMPETE_H
#define COMPETE_H

// Header
#include "header.h"
#include "structure.h"

// Compete Graph
class Compete{

	public:
		Compete();
		Compete(const vector<Link>&, const vector<TrancNode>&, const vector<InterNode>&, int);
		void initialize(const vector<Link>&, const vector<TrancNode>&, const vector<InterNode>&, int);
		void updateResource(const vector<Flow>&);
		void flowChangeList(const vector<Flow>&);
		void createGraph(const vector<Flow>&);
		~Compete();

	private:
		/* Flow pair for recording usage */
		class FlowPair{
			public:
				int flowID;
				int pathID;
				double traffic;
		};

	private:
		/* Resource node for compete graph checking */
		enum{ LINK_RES, TRANC_RES, INTER_RES };
		class CompRes{
			public:
				int srcID;				// Source switch ID
				int dstID;				// Destination switch ID (link only)
				int resType;			// Resource type
				int relCnt;				// Temporary release count
				int reqCnt;				// Temporary require count
				double resCap;			// Resource capacity
				bool isWireless;		// Check if wireless
				vector<int>iList;		// Interference nodes (wireless link only)
				vector<FlowPair>relList;
				vector<FlowPair>reqList;
		};
	
	private:
		/* Edge and Node for compete graph  */
		class CompEdge{
			public:
				int resID;
				int dstID;
		};
		class CompNode{
			public:
				int flowID;
				int pathID;
				vector<CompEdge>edge;
		};
		
	private:
		bool isInitLink;				// Record if link resource is initialized
		bool isInitTranc;				// Record if transceiver resource is initialized
		bool isInitInter;				// Record if interference resource is initialized
		map<int, int>trancMap;			// Map the index from src to transceiver node ID
		map<int, int>interMap;			// Map the index from src to interference node ID
		vector< map<int, int> >linkMap;	// Map the index from (src,dst) to link resource ID
		vector<CompRes>compRes;			// Record the remaining resource
		vector<CompNode>compNode;		// Compete graph
};

#endif

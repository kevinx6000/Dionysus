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
		bool needTemp(void);
		void changePlan(const vector<Link>&, const vector<Flow>&, vector<Flow>&, vector<Flow>&, int);
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
				vector<int>prev;
		};

	private:
		/* Enumeration var for vertex cover */
		enum{NOT_VISITED, BLACK, WHITE, VISITED};

	private:
		/* BFS node */
		class BFSNode{
			public:
				int switchID;
				map<int, double>interCap;
		};
	
	private:
		/* Resource difference node */
		class ResDiffNode{
			public:
				int ID;
				double reqTraffic;
				double relTraffic;
		};
		class ResDiff{
			public:
				vector<ResDiffNode>link;
				vector<ResDiffNode>tranc;
				vector<ResDiffNode>inter;
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

	private:
		/* For minimum vertex cover */
		int mvcSize;
		vector<int>mvcList;
		map<int, int>visMark;
		
	private:
		/* Private function */
		void backtrack(int, int, int);
		void occupyRes(const vector<Flow>&, int, int, int, double);
		void resDiffCheck(int, FlowPath&, ResDiff&);
		bool alterPath(const vector< vector<int> >&, const vector<CompRes>&, int, int, double, vector<Link>&);
		static bool cmpHop(Link, Link);
};

#endif

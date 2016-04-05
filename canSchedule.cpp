// Headers
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Check whether an operation is able to be scheduled
bool Dionysus::canSchedule(int owDpID){

	// Variables
	int owID, oaDpID, odDpID, pDpID, pID, rDpID, rID;
	int fID, fpID, sID, dID, portID, ingID, cID;
	bool canSchedule, isZero, isChanged;
	double total, avTmp, traffic;
	vector<double>preTraffic;
	vector<double>curTraffic;
	vector<double>commitTmp;
	vector<Rule>addRules;
	vector<Rule>delRules;
	Rule modRule, tmpRule;

	// We only check for weight change operations
	owID = nodes[owDpID].nodeIndex;
	if(nodes[owDpID].nodeType != OPERATION || 
			operations[owID].operationType != OP_MOD) return false;
	canSchedule = false;

	// Check link capacity resource
	total = 0.0;
	oaDpID = mapID[ nodes[owDpID].parent[0] ];
	for(int i = 0; i < (int)nodes[oaDpID].parent.size(); i++){

		// Path nodes
		pDpID = mapID[ nodes[oaDpID].parent[i] ];
		if(nodes[pDpID].nodeType == PATH){

			// Search for the operation child itself
			for(int j = 0; j < (int)nodes[pDpID].child.size(); j++)
				if(mapID[ nodes[pDpID].child[j].nodeID ] == oaDpID)
					avTmp = nodes[pDpID].child[j].dobWeight;

			// Resource nodes
			for(int j = 0; j < (int)nodes[pDpID].parent.size(); j++){

				// Link
				rDpID = mapID[ nodes[pDpID].parent[j] ];
				if(nodes[rDpID].nodeType == RES_LINK){
					
					// Remaining resource
					rID = nodes[rDpID].nodeIndex;
					avTmp = min(avTmp, links[rID].linkCapacity);

					// Link -> Path (only one link fits)
					for(int k = 0; k < (int)nodes[rDpID].child.size(); k++)
						if(mapID[ nodes[rDpID].child[k].nodeID ] == pDpID)
							avTmp = min(avTmp, nodes[rDpID].child[k].dobWeight);
				}
			}
			total += avTmp;
			paths[ nodes[pDpID].nodeIndex ].available = avTmp;
		}
	}

	// Some flow can be move right now
	if(total > 0.0) canSchedule = true;

	// We try to update traffic "virtually", and record the resulting rule set
	if(canSchedule){

		// Temporary committed traffic on each paths
		for(int i = 0; i < (int)nodes[owDpID].child.size(); i++){

			// Path nodes
			pDpID = mapID[ nodes[owDpID].child[i].nodeID ];
			if(nodes[pDpID].nodeType == PATH){

				// Committed traffic
				pID = nodes[pDpID].nodeIndex;
				commitTmp.push_back(min(nodes[owDpID].child[i].dobWeight, paths[ paths[pID].pairID ].available));
			}
		}

		// Previous Traffic
		fID = operations[ nodes[owDpID].nodeIndex ].flowID;
		for(int i = 0; i < (int)links.size(); i++){
			preTraffic.push_back(links[i].curTraffic[fID]);
			curTraffic.push_back(links[i].curTraffic[fID]);
		}

		// Update all path with new traffic distribution
		cID = 0;
		for(int i = 0; i < (int)nodes[owDpID].child.size(); i++){
			pDpID = mapID[ nodes[owDpID].child[i].nodeID ];
			if(nodes[pDpID].nodeType == PATH){
				pID = nodes[pDpID].nodeIndex;

				// Corresponding flow
				fpID = paths[pID].flowPathID;
				traffic = commitTmp[cID++];

				// New switches along the paths
				for(int j = 0; j < (int)allFlow[fID].flowPath[fpID].link[1].size(); j++){
					sID = allFlow[fID].flowPath[fpID].link[1][j].sourceID;
					dID = allFlow[fID].flowPath[fpID].link[1][j].destinationID;
					portID = findDstPort(sID, dID);

					// Update current state with committed traffic
					if(portID != -1)
						curTraffic[ switches[sID].linkID[portID] ] += traffic;

					// Exception: no such ID
					else{
						fprintf(stderr, "No such switch ID (%d) exisits from %d\n", dID, sID);
						exit(1);
					}
				}

				// Old switches along the paths
				for(int j = 0; j < (int)allFlow[fID].flowPath[fpID].link[0].size(); j++){
					sID = allFlow[fID].flowPath[fpID].link[0][j].sourceID;
					dID = allFlow[fID].flowPath[fpID].link[0][j].destinationID;
					portID = findDstPort(sID, dID);

					// Update current state with committed traffic + create new rule
					if(portID != -1)
						curTraffic[ switches[sID].linkID[portID] ] -= traffic;

					// Exception: no such ID
					else{
						fprintf(stderr, "No such switch ID (%d) exisits from %d\n", dID, sID);
						exit(1);
					}
				}

			}
		}

		// Ingress switch ID
		ingID = operations[ nodes[owDpID].nodeIndex ].switchID;

		// Add rule with new version
		for(int i = 0; i < (int)switches.size(); i++){
			if(i != ingID){

				// Check if links are changed OR all links are zero
				isZero = true, isChanged = false;
				for(int j = 0; j < (int)switches[i].linkID.size(); j++){
					traffic = curTraffic[ switches[i].linkID[j] ];
					if(traffic != 0.0) isZero = false;
					if(traffic != preTraffic[ switches[i].linkID[j] ]) isChanged = true;
				}

				// Install new rule if: 
				// 1. Link traffic of at least one link is not zero 
				// 2. Link traffic of at least one link has changed
				if(!isZero && isChanged){
					tmpRule.switchID = switches[i].switchID;
					tmpRule.ruleType = RULE_ADD;
					for(int j = 0; j < (int)switches[i].linkID.size(); j++)
						tmpRule.traffic.push_back(curTraffic[ switches[i].linkID[j] ]);
					addRules.push_back(tmpRule);
					tmpRule.traffic.clear();
				}
			}
		}

		// Modify ratio @ ingress switch
		modRule.switchID = ingID;
		modRule.ruleType = RULE_MOD;
		for(int i = 0; i < (int)switches[ingID].linkID.size(); i++)
			modRule.traffic.push_back(curTraffic[ switches[ingID].linkID[i] ]);

		// Delete rule with old version
		for(int i = 0; i < (int)switches.size(); i++){
			if(i != ingID){

				// Check if previous traffic is zero for all links
				isZero = true, isChanged = false;
				for(int j = 0; j < (int)switches[i].linkID.size(); j++){
					traffic = preTraffic[ switches[i].linkID[j] ];
					if(traffic != 0.0) isZero = false;
					if(traffic != curTraffic[ switches[i].linkID[j] ]) isChanged = true;
				}

				// Delete rule if
				// 1. Previous link traffic of at least one link is not zero
				// 2. Link traffic of at least one link has changed
				if(!isZero && isChanged){
					tmpRule.switchID = switches[i].switchID;
					tmpRule.ruleType = RULE_DEL;
					delRules.push_back(tmpRule);
				}
			}
		}
	}

	// Check switch memeory resource
	oaDpID = mapID[ nodes[owDpID].parent[0] ];
	for(int i = 0; i < (int)addRules.size(); i++){

		// Search switch resource ID
		rID = -1;
		for(int j = 0; j < (int)nodes[oaDpID].parent.size(); j++){
			rDpID = mapID[ nodes[oaDpID].parent[j] ];
			if(nodes[rDpID].nodeType == RES_SWITCH){
				rID = nodes[rDpID].nodeIndex;
				if(switches[rID].switchID == addRules[i].switchID) break;
			}
		}
		if(rID != -1 && switches[rID].tcamUsage < 1) canSchedule = false;
	}

	// Both links & switches are enough
	if(canSchedule){

		// Update link capacity resource
		oaDpID = mapID[ nodes[owDpID].parent[0] ];
		for(int i = 0; i < (int)nodes[oaDpID].parent.size(); i++){

			// Path nodes
			pDpID = mapID[ nodes[oaDpID].parent[i] ];
			if(nodes[pDpID].nodeType == PATH){

				// Resource nodes
				avTmp = paths[ nodes[pDpID].nodeIndex ].available;
				for(int j = 0; j < (int)nodes[pDpID].parent.size(); j++){

					// Link resource
					rDpID = mapID[ nodes[pDpID].parent[j] ];
					if(nodes[rDpID].nodeType == RES_LINK){

						// Link capacity
						rID = nodes[rDpID].nodeIndex;
						links[rID].linkCapacity -= avTmp;

						// Link -> Path (only one link fits)
						for(int k = 0; k < (int)nodes[rDpID].child.size(); k++)
							if(mapID[ nodes[rDpID].child[k].nodeID ] == pDpID)
								nodes[rDpID].child[k].dobWeight -= avTmp;
					}
				}

				// Path -> Operation
				for(int j = 0; j < (int)nodes[pDpID].child.size(); j++)
					if(mapID[ nodes[pDpID].child[j].nodeID ] == oaDpID)
						nodes[pDpID].child[j].dobWeight -= avTmp;
			}
		}
		
		// Update switch memory resource
		oaDpID = mapID[ nodes[owDpID].parent[0] ];
		for(int i = 0; i < (int)addRules.size(); i++){

			// Search switch resource ID
			rID = -1;
			for(int j = 0; j < (int)nodes[oaDpID].parent.size(); j++){
				rDpID = mapID[ nodes[oaDpID].parent[j] ];
				if(nodes[rDpID].nodeType == RES_SWITCH){
					rID = nodes[rDpID].nodeIndex;
					if(switches[rID].switchID == addRules[i].switchID) break;
				}
			}
			if(rID != -1) switches[rID].tcamUsage--;
		}

		// Update the children of change weight operation
		cID = 0;
		for(int i = 0; i < (int)nodes[owDpID].child.size(); i++){

			// Path nodes
			pDpID = mapID[ nodes[owDpID].child[i].nodeID ];
			if(nodes[pDpID].nodeType == PATH){

				// Committed traffic
				pID = nodes[pDpID].nodeIndex;
				paths[pID].committed = commitTmp[cID++];
				nodes[owDpID].child[i].dobWeight -= paths[pID].committed;
				total -= paths[pID].committed;
			}
		}

		// Push all three ruleset into operation nodes
		oaDpID = mapID[ nodes[owDpID].parent[0] ];
		operations[ nodes[oaDpID].nodeIndex ].ruleSet = addRules;
		operations[ nodes[owDpID].nodeIndex ].ruleSet.clear();
		operations[ nodes[owDpID].nodeIndex ].ruleSet.push_back(modRule);
		for(int i = 0; i < (int)nodes[owDpID].child.size(); i++){
			if(nodes[ mapID[ nodes[owDpID].child[i].nodeID ] ].nodeType == OPERATION){
				odDpID = mapID[ nodes[owDpID].child[i].nodeID ];
				break;
			}
		}
		operations[ nodes[odDpID].nodeIndex ].ruleSet = delRules;
	}
	
	// Return can be scheduling or not
	return canSchedule;
}

// Headers
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Check whether an operation is able to be scheduled
bool Dionysus::canSchedule(int owDpID){

	// Variables
	int owID, oaDpID, odDpID, pDpID, pID, rDpID, rID;
	int fID, fpID, sID, dID, portID, ingID, cID;
	int ratio;
	bool canSchedule, isChanged, hasPathParent;
	double total, avTmp, traffic, preTotal, curTotal;
	vector<double>preTraffic;
	vector<double>curTraffic;
	vector<double>commitTmp;
	vector<Rule>addRules;
	vector<Rule>delRules;
	Rule modRule, tmpRule;

	// We only check for weight change operations
	owID = nodes[owDpID].nodeIndex;
	if(nodes[owDpID].nodeType != OPERATION || 
			operations[owID].operationType != OP_MOD ||
			operations[owID].isFinished) return false;
	canSchedule = false;

	// Check link capacity resource
	total = 0.0;
	hasPathParent = false;
	oaDpID = mapID[ nodes[owDpID].parent[0] ];
	for(int i = 0; i < (int)nodes[oaDpID].parent.size(); i++){

		// Path nodes
		pDpID = mapID[ nodes[oaDpID].parent[i] ];
		if(nodes[pDpID].nodeType == PATH){

			// At least has one path parent
			hasPathParent = true;

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
	if(!hasPathParent) operations[ nodes[owDpID].nodeIndex ].isFinished = true;

	// Check transceiver and interference node
	if(canSchedule){

		// Reset to false, and check whether at least one path can be updated
		total = 0.0;
		canSchedule = false;
		oaDpID = mapID[ nodes[owDpID].parent[0] ];
		for(int i = 0; i < (int)nodes[oaDpID].parent.size(); i++){

			// Path nodes
			pDpID = mapID[ nodes[oaDpID].parent[i] ];
			if(nodes[pDpID].nodeType == PATH){

				// Minimum available traffic
				avTmp = paths[ nodes[pDpID].nodeIndex ].available;

				// Resource nodes
				for(int j = 0; j < (int)nodes[pDpID].parent.size(); j++){

					// Transceiver
					rDpID = mapID[ nodes[pDpID].parent[j] ];
					if(nodes[rDpID].nodeType == RES_TRANC){

						// Find out ratio: Link -> Path (only one fit)
						rID = nodes[rDpID].nodeIndex;
						for(int k = 0; k < (int)nodes[rDpID].child.size(); k++)
							if(mapID[ nodes[rDpID].child[k].nodeID ] == pDpID)
								ratio = nodes[rDpID].child[k].intWeight;

						// Remaining resource
						avTmp = min(avTmp, trancNode[ nodes[rDpID].nodeIndex ].nodeCapacity/ratio);

						// All current demand
						for(int k = 0; k < (int)nodes[rDpID].child.size(); k++)
							if(mapID[ nodes[rDpID].child[k].nodeID ] == pDpID)
								avTmp = min(avTmp, nodes[rDpID].child[k].dobWeight/ratio);
					}

					// Interference
					if(nodes[rDpID].nodeType == RES_INTER){

						// Find out ratio: Link -> Path (only one fit)
						rID = nodes[rDpID].nodeIndex;
						for(int k = 0; k < (int)nodes[rDpID].child.size(); k++)
							if(mapID[ nodes[rDpID].child[k].nodeID ] == pDpID)
								ratio = nodes[rDpID].child[k].intWeight;

						// Remaining resource
						avTmp = min(avTmp, interNode[ nodes[rDpID].nodeIndex ].nodeCapacity/ratio);

						// All current demand
						for(int k = 0; k < (int)nodes[rDpID].child.size(); k++)
							if(mapID[ nodes[rDpID].child[k].nodeID ] == pDpID)
								avTmp = min(avTmp, nodes[rDpID].child[k].dobWeight/ratio);
					}
				}

				// Record and write back
				total += avTmp;
				paths[ nodes[pDpID].nodeIndex ].available = avTmp;
			}
		}

		// Some flow can be move right now
		if(total > 0.0) canSchedule = true;
	}

	// We try to update traffic "virtually", and record the resulting rule set
	if(canSchedule){

		// Temporary committed traffic on each paths
		for(int i = 0; i < (int)nodes[owDpID].child.size(); i++){
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

				// Corresponding flow
				pID = nodes[pDpID].nodeIndex;
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

				// Check if ratio are changed OR all links are zero
				isChanged = false;
				preTotal = curTotal = 0.0;
				for(int j = 0; j < (int)switches[i].linkID.size(); j++){
					curTotal += curTraffic[ switches[i].linkID[j] ];
					preTotal += preTraffic[ switches[i].linkID[j] ];
				}

				// At least one current link is not zero
				if(curTotal > 0.0){

					// From zero to non-zero: must changed!
					if(preTotal == 0.0) isChanged = true;
					else{
						// Check ratio difference
						for(int j = 0; j < (int)switches[i].linkID.size(); j++){
							if(curTraffic[ switches[i].linkID[j] ]/curTotal != preTraffic[ switches[i].linkID[j] ]/preTotal){
								isChanged = true;
								break;
							}
						}
					}
				}

				// Install new rule if: 
				// 1. Link traffic of at least one current link is not zero 
				// 2. Link traffic ratio of at least one link has changed
				if(curTotal > 0.0 && isChanged){
					tmpRule.switchID = switches[i].switchID;
					tmpRule.ruleType = RULE_ADD;
					for(int j = 0; j < (int)switches[i].linkID.size(); j++)
						tmpRule.traffic.push_back(curTraffic[ switches[i].linkID[j] ]);
					addRules.push_back(tmpRule);
					tmpRule.traffic.clear();
				}
			}
		}

		// Delete rule with old version
		for(int i = 0; i < (int)switches.size(); i++){
			if(i != ingID){

				// Check if previous traffic is zero for all links
				isChanged = false;
				preTotal = curTotal = 0.0;
				for(int j = 0; j < (int)switches[i].linkID.size(); j++){
					curTotal += curTraffic[ switches[i].linkID[j] ];
					preTotal += preTraffic[ switches[i].linkID[j] ];
				}

				// At least one previous link is not zero
				if(preTotal > 0.0){

					// From non-zero to zero: must changed!
					if(curTotal == 0.0) isChanged = true;
					else{
						// Check ratio difference
						for(int j = 0; j < (int)switches[i].linkID.size(); j++){
							if(curTraffic[ switches[i].linkID[j] ]/curTotal != preTraffic[ switches[i].linkID[j] ]/preTotal){
								isChanged = true;
								break;
							}
						}
					}
				}

				// Delete rule if
				// 1. Link traffic of at least one previous link is not zero
				// 2. Link traffic ratio of at least one link has changed
				if(preTotal > 0.0 && isChanged){
					tmpRule.switchID = switches[i].switchID;
					tmpRule.ruleType = RULE_DEL;
					delRules.push_back(tmpRule);
				}
			}
		}

		// Modify ratio @ ingress switch
		preTotal = curTotal = 0.0;
		for(int i = 0; i < (int)switches[ingID].linkID.size(); i++){
			curTotal += curTraffic[ switches[ingID].linkID[i] ];
			preTotal += preTraffic[ switches[ingID].linkID[i] ];
		}
		isChanged = false;
		for(int i = 0; i < (int)switches[ingID].linkID.size(); i++){
			if(curTraffic[ switches[ingID].linkID[i] ]/curTotal != preTraffic[ switches[ingID].linkID[i] ]/preTotal){
				isChanged = true;
				break;
			}
		}
		if(isChanged || addRules.size() > 0 || delRules.size() > 0){
			modRule.switchID = ingID;
			modRule.ruleType = RULE_MOD;
			for(int i = 0; i < (int)switches[ingID].linkID.size(); i++)
				modRule.traffic.push_back(curTraffic[ switches[ingID].linkID[i] ]);
		}
		else modRule.switchID = -1;
	}

	// Check switch memeory resource
	oaDpID = mapID[ nodes[owDpID].parent[0] ];
	for(int i = 0; i < (int)addRules.size(); i++){

		// Search switch resource ID
		rID = -1;
		for(int j = 0; j < (int)nodes[oaDpID].parent.size(); j++){
			rDpID = mapID[ nodes[oaDpID].parent[j] ];
			if(nodes[rDpID].nodeType == RES_SWITCH && 
				switches[nodes[rDpID].nodeIndex].switchID == addRules[i].switchID){
					rID = nodes[rDpID].nodeIndex;
					break;
			}
		}
		if(rID != -1 && switches[rID].tcamUsage < 1) canSchedule = false;
	}

	// All links, switches, transceiver, interference are enough
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

		// Update transceiver and interference resource
		oaDpID = mapID[ nodes[owDpID].parent[0] ];
		for(int i = 0; i < (int)nodes[oaDpID].parent.size(); i++){

			// Path nodes
			pDpID = mapID[ nodes[oaDpID].parent[i] ];
			if(nodes[pDpID].nodeType == PATH){

				// Minimum available traffic
				avTmp = paths[ nodes[pDpID].nodeIndex ].available;

				// Resource nodes
				for(int j = 0; j < (int)nodes[pDpID].parent.size(); j++){

					// Transceiver
					rDpID = mapID[ nodes[pDpID].parent[j] ];
					if(nodes[rDpID].nodeType == RES_TRANC){

						// Find out ratio: Link -> Path (only one fit)
						rID = nodes[rDpID].nodeIndex;
						for(int k = 0; k < (int)nodes[rDpID].child.size(); k++)
							if(mapID[ nodes[rDpID].child[k].nodeID ] == pDpID)
								ratio = nodes[rDpID].child[k].intWeight;

						// Remaining resource
						trancNode[rID].nodeCapacity -= avTmp * ratio;

						// All current demand
						for(int k = 0; k < (int)nodes[rDpID].child.size(); k++)
							if(mapID[ nodes[rDpID].child[k].nodeID ] == pDpID)
								nodes[rDpID].child[k].dobWeight -= avTmp * ratio;
					}

					// Interference
					if(nodes[rDpID].nodeType == RES_INTER){

						// Find out ratio: Link -> Path (only one fit)
						rID = nodes[rDpID].nodeIndex;
						for(int k = 0; k < (int)nodes[rDpID].child.size(); k++)
							if(mapID[ nodes[rDpID].child[k].nodeID ] == pDpID)
								ratio = nodes[rDpID].child[k].intWeight;

						// Remaining resource
						interNode[rID].nodeCapacity -= avTmp * ratio;

						// All current demand
						for(int k = 0; k < (int)nodes[rDpID].child.size(); k++)
							if(mapID[ nodes[rDpID].child[k].nodeID ] == pDpID)
								nodes[rDpID].child[k].dobWeight -= avTmp * ratio;
					}
				}
			}
		}
		
		// Update switch memory resource
		oaDpID = mapID[ nodes[owDpID].parent[0] ];
		for(int i = 0; i < (int)addRules.size(); i++){

			// Search switch resource ID
			rID = -1;
			for(int j = 0; j < (int)nodes[oaDpID].parent.size(); j++){
				rDpID = mapID[ nodes[oaDpID].parent[j] ];
				if(nodes[rDpID].nodeType == RES_SWITCH &&
					switches[nodes[rDpID].nodeIndex].switchID == addRules[i].switchID){
						rID = nodes[rDpID].nodeIndex;
						break;
				}
			}
			if(rID != -1) switches[rID].tcamUsage--;
		}

		// Record the committed traffic
		cID = 0;
		for(int i = 0; i < (int)nodes[owDpID].child.size(); i++){

			// Path nodes
			pDpID = mapID[ nodes[owDpID].child[i].nodeID ];
			if(nodes[pDpID].nodeType == PATH){

				// Committed traffic
				pID = nodes[pDpID].nodeIndex;
				paths[pID].committed = commitTmp[cID++];
			}
		}

		// Copy back the latest traffic
		fID = operations[ nodes[owDpID].nodeIndex ].flowID;
		for(int i = 0; i < (int)links.size(); i++)
			links[i].curTraffic[fID] = curTraffic[i];

		// Operation IDs
		oaDpID = mapID[ nodes[owDpID].parent[0] ];
		for(int i = 0; i < (int)nodes[owDpID].child.size(); i++){
			if(nodes[ mapID[ nodes[owDpID].child[i].nodeID ] ].nodeType == OPERATION){
				odDpID = mapID[ nodes[owDpID].child[i].nodeID ];
				break;
			}
		}

		// Clear previous operation ruleset
		operations[ nodes[oaDpID].nodeIndex ].ruleSet.clear();
		operations[ nodes[owDpID].nodeIndex ].ruleSet.clear();
		operations[ nodes[odDpID].nodeIndex ].ruleSet.clear();

		// Push all three ruleset into operation nodes
		// (if at least one ratio @ ingress switch changed, or new rules added / old rules deleted)
		if(modRule.switchID != -1){
			operations[ nodes[oaDpID].nodeIndex ].ruleSet = addRules;
			operations[ nodes[owDpID].nodeIndex ].ruleSet.push_back(modRule);
			operations[ nodes[odDpID].nodeIndex ].ruleSet = delRules;
		}
	}
	
	// Return can be scheduling or not
	return canSchedule;
}

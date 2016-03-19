#ifndef GEN_DEP_GRAPH_H
#define GEN_DEP_GRAPH_H

// Headers
#include "header.h"
#include "structure.h"

// Create node
int createNode(DPGraph &dpGraph, int nodeType, int nodeIndex = 0){
	Node ntmp;
	ntmp.nodeID = dpGraph.nodes.size();
	ntmp.nodeType = nodeType;
	ntmp.nodeIndex = nodeIndex;
	dpGraph.nodes.push_back(ntmp);
	return ntmp.nodeID;
}

// Generate Dependency Graph
void genDependencyGraph(DPGraph &dpGraph, const vector<Flow> &allFlows, 
		vector<Switch> &switches, vector<Link> &links, vector<Operation> &operations){

	// Variables
	int pInID;
	int pOutID;
	int ptr1, ptr2, siz1, siz2, sID1, sID2;
	int rID, ingID1, ingID2;
	bool diff;
	double vol1, vol2;
	Edge etmp;
	Operation otmp;
	vector<int>addOpID;
	vector<int>delOpID;
	otmp.isFinished = false;

	// Resource Node: switch
	for(int i = 0; i < switches.size(); i++)
		switches[i].dpID = createNode(dpGraph, RES_SWITCH, i);

	// Resource Node: links
	for(int i = 0; i < links.size(); i++)
		links[i].dpID = createNode(dpGraph, RES_LINK, i);

	// For each flow
	for(int i = 0; i < allFlows.size(); i++){

		// Create path nodes
		pInID  = createNode(dpGraph, PATH);
		pOutID = createNode(dpGraph, PATH);

		// Compare initial & final distribution
		ptr1 = ptr2 = 0;
		siz1 = allFlows[i].traffic[0].size();
		siz2 = allFlows[i].traffic[1].size();
		while(ptr1 < siz1 && ptr2 < siz2){

			// Switch ID of two pointers
			sID1 = allFlows[i].traffic[0][ptr1].switchID;
			sID2 = allFlows[i].traffic[1][ptr2].switchID;

			// Ingress switch
			if(sID1 == allFlows[i].ingressID){
				ingID1 = ptr1, ptr1++;
				continue;
			}
			if(sID2 == allFlows[i].ingressID){
				ingID2 = ptr2, ptr2++;
				continue;
			}

			// Same switch
			if(sID1 == sID2){

				// Check difference
				diff = false;
				for(int j = 0; j < switches[sID1].port.size(); j++){
					vol1 = allFlows[i].traffic[0][ptr1].volume[j];
					vol2 = allFlows[i].traffic[1][ptr2].volume[j];

					// At least one link with different traffic volume
					if(vol1 != vol2){
						diff = true;
						rID = links[ switches[sID1].linkID[j] ].dpID;

						// Add link in dependency graph: Link -> PathIn
						if(vol1 < vol2){
							etmp.nodeID = pInID;
							etmp.dobWeight = vol2 - vol1;
							dpGraph.nodes[rID].child.push_back(etmp);
							dpGraph.nodes[pInID].parent.push_back(rID);
						}

						// Add link in dependency graph: PathOut -> Link
						else{
							etmp.nodeID = rID;
							etmp.dobWeight = vol1 - vol2;
							dpGraph.nodes[pOutID].child.push_back(etmp);
						}
					}
				}

				// At least one link with different traffic volume
				if(diff){

					/* Add new rule */

					// Create operation node
					otmp.dpID = createNode(dpGraph, OPERATION, operations.size());
					otmp.operationType = OP_ADD;
					operations.push_back(otmp);
					addOpID.push_back(otmp.dpID);

					// Add link in dependency graph: Switch -> Operation
					etmp.nodeID = otmp.dpID;
					etmp.intWeight = 1;
					dpGraph.nodes[ switches[sID1].dpID ].child.push_back(etmp);
					dpGraph.nodes[ otmp.dpID ].parent.push_back(switches[sID1].dpID);

					// Add link in dependency graph: Path -> Operation
					etmp.intWeight = 0; /* NOT KNOWING WHY WEIGHT IS REQUIRED */
					dpGraph.nodes[ pInID ].child.push_back(etmp);
					dpGraph.nodes[ otmp.dpID ].parent.push_back(pInID);

					/* Remove old rule */

					// Create operation node
					otmp.dpID = createNode(dpGraph, OPERATION, operations.size());
					otmp.operationType = OP_DEL;
					operations.push_back(otmp);
					delOpID.push_back(otmp.dpID);

					// Add link in dependency graph: Operation -> Switch
					etmp.nodeID = switches[ sID1 ].dpID;
					etmp.intWeight = 1;
					dpGraph.nodes[ otmp.dpID ].child.push_back(etmp);
				}

				// Update pointers
				ptr1++, ptr2++;
			}

			// Switch rule deleted: sID1
			else if(sID1 < sID2){

				// Create operation node: in operation vector
				otmp.dpID = createNode(dpGraph, OPERATION, operations.size());
				otmp.operationType = OP_DEL;
				operations.push_back(otmp);
				delOpID.push_back(otmp.dpID);

				// Add link in dependency graph: Operation -> Switch
				etmp.nodeID = switches[ sID1 ].dpID;
				etmp.intWeight = 1;
				dpGraph.nodes[ otmp.dpID ].child.push_back(etmp);

				// Add link in dependency graph: Path -> Link
				for(int j = 0; j < switches[sID1].port.size(); j++){
					if(allFlows[i].traffic[0][ptr1].volume[j] > 0){
						etmp.nodeID = links[ switches[sID1].linkID[j] ].dpID;
						etmp.dobWeight = allFlows[i].traffic[0][ptr1].volume[j];
						dpGraph.nodes[pOutID].child.push_back(etmp);
					}
				}

				// Update pointer
				ptr1++;
			}

			// Switch rule added: sID2
			else{ /* sID1 > sID2 */

				// Create operation node
				otmp.dpID = createNode(dpGraph, OPERATION, operations.size());
				otmp.operationType = OP_ADD;
				operations.push_back(otmp);
				addOpID.push_back(otmp.dpID);

				// Add link in dependency graph: Switch -> Operation
				etmp.nodeID = otmp.dpID;
				etmp.intWeight = 1;
				dpGraph.nodes[ switches[sID2].dpID ].child.push_back(etmp);
				dpGraph.nodes[ otmp.dpID ].parent.push_back( switches[sID2].dpID );

				// Add link in dependency graph: Path -> Operation
				etmp.intWeight = 0; /* NOT KNOWING WHY WEIGHT IS REQUIRED */
				dpGraph.nodes[ pInID ].child.push_back(etmp);
				dpGraph.nodes[ otmp.dpID ].parent.push_back( pInID );

				// Add link in dependency graph: Link -> Path
				for(int j = 0; j < switches[sID2].port.size(); j++){
					if(allFlows[i].traffic[1][ptr2].volume[j] > 0){
						etmp.nodeID = pInID;
						etmp.dobWeight = allFlows[i].traffic[1][ptr2].volume[j];
						dpGraph.nodes[links[ switches[sID2].linkID[j] ].dpID].child.push_back(etmp);
						dpGraph.nodes[ pInID ].parent.push_back( links[switches[sID2].linkID[j]].dpID );
					}
				}

				// Update pointer
				ptr2++;
			}
		}

		// Old deleted rules
		while(ptr1 < siz1){
			sID1 = allFlows[i].traffic[0][ptr1].switchID;

			// Create operation node: in operation vector
			otmp.dpID = createNode(dpGraph, OPERATION, operations.size());
			otmp.operationType = OP_DEL;
			operations.push_back(otmp);
			delOpID.push_back(otmp.dpID);

			// Add link in dependency graph: Operation -> Switch
			etmp.nodeID = switches[ sID1 ].dpID;
			etmp.intWeight = 1;
			dpGraph.nodes[ otmp.dpID ].child.push_back(etmp);

			// Add link in dependency graph: Path -> Link
			for(int j = 0; j < switches[sID1].port.size(); j++){
				etmp.nodeID = links[ switches[sID1].linkID[j] ].dpID;
				etmp.dobWeight = vol1;
				dpGraph.nodes[pOutID].child.push_back(etmp);
			}
			ptr1++;
		}

		// New added rules
		while(ptr2 < siz2){
			sID2 = allFlows[i].traffic[1][ptr2].switchID;

			// Create operation node: in operation vector
			otmp.dpID = createNode(dpGraph, OPERATION, operations.size());
			otmp.operationType = OP_ADD;
			operations.push_back(otmp);
			addOpID.push_back(otmp.dpID);

			// Add link in dependency graph: Switch -> Operation
			etmp.nodeID = otmp.dpID;
			etmp.intWeight = 1;
			dpGraph.nodes[ switches[sID2].dpID ].child.push_back(etmp);
			dpGraph.nodes[ otmp.dpID ].parent.push_back( switches[sID2].dpID );

			// Add link in dependency graph: Path -> Operation
			etmp.intWeight = 0; /* NOT KNOWING WHY WEIGHT IS REQUIRED */
			dpGraph.nodes[ pInID ].child.push_back(etmp);
			dpGraph.nodes[ otmp.dpID ].parent.push_back( pInID );

			// Add link in dependency graph: Link -> Path
			for(int j = 0; j < switches[sID2].port.size(); j++){
				etmp.nodeID = pInID;
				etmp.dobWeight = vol2;
				dpGraph.nodes[links[ switches[sID2].linkID[j] ].dpID].child.push_back(etmp);
				dpGraph.nodes[ pInID ].parent.push_back( links[switches[sID2].linkID[j]].dpID );
			}
			ptr2++;
		}

		// Ingress switch: modify (add new rule)
		otmp.dpID = createNode(dpGraph, OPERATION, operations.size());
		otmp.operationType = OP_MOD;
		operations.push_back(otmp);
		/* Add op -> Mod op */
		etmp.nodeID = otmp.dpID;
		for(int j = 0; j < addOpID.size(); j++){
			dpGraph.nodes[ addOpID[j] ].child.push_back(etmp);
			dpGraph.nodes[ otmp.dpID ].parent.push_back( addOpID[j] );
		}
		/* Mod op -> Del op */
		for(int j = 0; j < delOpID.size(); j++){
			etmp.nodeID = delOpID[j];
			dpGraph.nodes[ otmp.dpID ].child.push_back(etmp);
			dpGraph.nodes[ delOpID[j] ].parent.push_back( otmp.dpID );
		}
		/* Mod op -> Path */
		etmp.nodeID = pOutID;
		dpGraph.nodes[ otmp.dpID ].child.push_back(etmp);
		dpGraph.nodes[ pOutID ].parent.push_back( otmp.dpID );

		/* Link -> Path or Path -> Link */
		for(int j = 0; j < switches[ allFlows[i].ingressID ].port.size(); j++){
			vol1 = allFlows[i].traffic[0][ingID1].volume[j];
			vol2 = allFlows[i].traffic[1][ingID2].volume[j];
			rID = links[ switches[ allFlows[i].ingressID ].linkID[j] ].dpID;

			// Add link in dependency graph: Link -> PathIn
			if(vol1 < vol2){
				etmp.nodeID = pInID;
				etmp.dobWeight = vol2 - vol1;
				dpGraph.nodes[ rID ].child.push_back(etmp);
				dpGraph.nodes[ pInID ].parent.push_back(rID);
			}

			// Add link in dependency graph: PathOut -> Link
			else{
				etmp.nodeID = rID;
				etmp.dobWeight = vol1 - vol2;
				dpGraph.nodes[pOutID].child.push_back(etmp);
			}
		}
		
		// Clear operation index
		addOpID.clear();
		delOpID.clear();
	}
}


#endif

// Header
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Sort CPL
void Dionysus::sortCPL(void){

	// Sort according to CPL
	sort(nodes.begin(), nodes.end(), cmpCPL);

	// Re-map the node ID
	for(int i = 0; i < (int)nodes.size(); i++)
		mapID[ nodes[i].nodeID ] = i;
}

// Sort comparison function for Node with CPL
bool Dionysus::cmpCPL(Node A, Node B){
	return A.CPL > B.CPL;
}

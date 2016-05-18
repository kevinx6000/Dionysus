// Header
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Reset all DC except topo
void Dionysus::reset(void){
	
	/* Resource nodes */
	for(int i = 0; i < (int)switches.size(); i++)
		switches[i].tcamUsage = TCAM_CAPACITY;
	for(int i = 0; i < (int)links.size(); i++){
		links[i].linkCapacity = LINK_CAPACITY;
		links[i].curTraffic.clear();
	}
	for(int i = 0; i < (int)trancNode.size(); i++)
		trancNode[i].nodeCapacity = LINK_CAPACITY;
	for(int i = 0; i < (int)interNode.size(); i++)
		interNode[i].nodeCapacity = LINK_CAPACITY;

	/* Dependency graph */
	paths.clear();
	operations.clear();
	nodes.clear();
	mapID.clear();

	/* Compete graph? */
}

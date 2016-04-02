// Header
#include "dionysus.h"

// Main
int main(void)
{
	// Simulator
	Dionysus simulator;

	// Read input
	simulator.readTopo();
	simulator.readFlow();

	// Generate dependency graph
	simulator.genDepGraph();

	// Update dependency graph
	simulator.updateGraph();

	// Cycle exits when calculating CPL
	if(!simulator.calculateCPL()) fprintf(stderr, "ERROR: cycle exists\n");
	else{

		// Sort with their CPL
		simulator.sortCPL();
	}

	// DEBUG
	simulator.debug();

	return 0;
}

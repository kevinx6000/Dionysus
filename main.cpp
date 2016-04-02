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

	// Calculate CPL
	if(!simulator.calculateCPL()) fprintf(stderr, "ERROR: cycle exists\n");

	// Debug
	else
	simulator.debug();

	return 0;
}

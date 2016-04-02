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

	// Schedule update
	simulator.schedule();

	// DEBUG
	simulator.debug();

	return 0;
}

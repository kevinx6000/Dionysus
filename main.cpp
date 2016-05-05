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

	// Check cycling/chaining effect
	simulator.checkCompete();

	// Generate dependency graph
	simulator.genDepGraph();

	// DEBUG
	simulator.debug();

	// Schedule update
	simulator.start();

	// DEBUG
	simulator.debug();

	return 0;
}

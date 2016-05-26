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

	// DEBUG
	simulator.debug();

	// Schedule update
	simulator.start();

	// DEBUG
	simulator.debug();

	// Statistic result
	simulator.statistic();

	return 0;
}

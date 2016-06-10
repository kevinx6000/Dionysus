// Header
#include "dionysus.h"

// Main
int main(void)
{
	// Random seed
	srand((unsigned)clock());

	// Simulator
	Dionysus simulator;

	// Read input
	simulator.readTopo();
	simulator.readFlow();

	// Pre-process wireless shortest path
	simulator.wirelessPath();

	// DEBUG
	simulator.debug();

	// Start simulation
	simulator.start();

	// DEBUG
	simulator.debug();

	// Statistic result
	simulator.statistic();

	return 0;
}

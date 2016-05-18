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

	// DEBUG
	simulator.debug();

	// Start simulation
	simulator.start();

	// DEBUG
	simulator.debug();

	return 0;
}

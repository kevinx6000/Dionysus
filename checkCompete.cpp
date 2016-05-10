// Header
#include "header.h"
#include "structure.h"
#include "dionysus.h"
#include "compete.h"

// Check if chaining/Cycling exist
bool Dionysus::checkCompete(void){

	// Variable
	bool hasCompete = false;
	
	// Initialize compete graph checker
	this->compete.initialize(links, trancNode, interNode, pod);

	// Update initial remaining resource
	this->compete.updateResource(allFlow);

	return hasCompete;
}

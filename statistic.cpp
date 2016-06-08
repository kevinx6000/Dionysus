// Header
#include "header.h"
#include "structure.h"
#include "dionysus.h"

// Statistic report
void Dionysus::statistic(void){
	fprintf(stderr, "[Result]\n");
	fprintf(stderr, "Add rules = %d\n", addRuleCnt);
	fprintf(stderr, "Mod rules = %d\n", modRuleCnt);
	fprintf(stderr, "Del rules = %d\n", delRuleCnt);
	fprintf(stderr, "Round = %d\n", roundCnt);
}

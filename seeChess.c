// seeChess.c

#include "stdio.h"
#include "defs.h"
#include "stdlib.h"

#define WAC1 "8/8/5k2/8/2P5/3K4/8/8 w - - 0 1"

int main() {

	AllInit();
	Uci_Loop();

	return 0;
}

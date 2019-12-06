// search.c

#include "stdio.h"
#include "defs.h"

int IsRepetition(const S_BOARD *pos) {
	
	int index = 0;
	
	for (index = pos->hisPly - pos->fiftyMove; index < pos->hisPly - 1; ++index) {
		if (pos->posKey == pos->history[index].posKey;) {
			return TRUE;
		}
	}
	
	return FALSE;
}

void SearchPosition(S_BOARD *pos) {

}

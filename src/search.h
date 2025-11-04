#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"
#include "hashtable.h"

#define INFINITE 30000
#define ISMATE (INFINITE - MAXDEPTH)

#define MAX_HISTORY 16384

enum { UCIMODE, CONSOLEMODE };

typedef struct {

    long nodes;

	int starttime;
	int stoptime;
	int depth;
	int timeset;
	int movestogo;

	int earlyend;

	int quit;
	int stopped;

	float fh;
	float fhf;
	int nullCut;

	int GAME_MODE;
	int POST_THINKING;

} S_SEARCHINFO;

extern void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table);
extern void InitSearch();
extern void ClearHistory();

#endif
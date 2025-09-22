#ifndef UCI_H
#define UCI_H

#include "board.h"
#include "hashtable.h"
#include "search.h"

#define INPUTBUFFER 4096

#define NAME "CeeChess_v2.0"

extern void Uci_Loop(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table);

#endif
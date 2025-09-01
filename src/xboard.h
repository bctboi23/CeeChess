#ifndef XBOARD_H
#define XBOARD_H

#include "board.h"
#include "hashtable.h"
#include "search.h"

extern void XBoard_Loop(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table);
extern void Console_Loop(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table);

#endif
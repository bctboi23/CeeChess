#ifndef CONSOLE_H
#define CONSOLE_H

#include "board.h"
#include "hashtable.h"
#include "search.h"

extern void Console_Loop(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table);

#endif
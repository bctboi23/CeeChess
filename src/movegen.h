#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "board.h"
#include "move.h"

extern void GenerateAllMoves(const S_BOARD *pos, S_MOVELIST *list);
extern void GenerateAllCaps(const S_BOARD *pos, S_MOVELIST *list);

extern int MoveExists(S_BOARD *pos, const int move);

extern void InitMvvLva();

#endif
#ifndef ATTACK_H
#define ATTACK_H
#include "board.h"

extern int SqAttacked(const int sq, const int side, const S_BOARD *pos);
extern U64 getAttacks(const int sq, const int side, const S_BOARD *pos);
extern U64 discoveredAttackExists(const int sq, const int side, const S_BOARD *pos);
extern U64 getPawnAttackSpan(int sq, int color);

#endif
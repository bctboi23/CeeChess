#ifndef BITBOARDS_H
#define BITBOARDS_H

#include <stdint.h>

#define LOOKUP_TABLE_SIZE 88772

#define POP_LSB(b) popLSB(b)
#define POP_CNT(b) __builtin_popcountll(b)
#define CLRBIT(bb,sq) ((bb) &= ClearMask[(sq)])
#define SETBIT(bb,sq) ((bb) |= SetMask[(sq)])
#define CHKBIT(bb,sq) (testBit(bb, sq))

typedef uint64_t U64;

extern U64 SetMask[64];
extern U64 ClearMask[64];

extern U64 FileBBMask[8];
extern U64 RankBBMask[8];

extern U64 ForwardRanksMasks[2][8];

extern U64 BlackPassedMask[64];
extern U64 WhitePassedMask[64];
extern U64 PassedMask[2][64];

extern U64 IsolatedMask[64];

extern U64 BlackConnectedMask[64];
extern U64 WhiteConnectedMask[64];
extern U64 ConnectedMask[2][64];

extern U64 KingAreaMasks[2][64];
extern U64 PawnShieldMasks[2][64];

extern U64 nonSliderMoveTable[2][64];
extern U64 pawnAttackTable[2][64];

extern void initMasks(int bishop);
extern void initAttackTable(int bishop);
extern void InitPawnAttackLookup();
extern void InitNonSlideMoveLookup();
extern void printBitBoard(U64 bb);

extern int popLSB(U64 *bb);
extern int testBit(U64 bb, int sq);

extern int several(U64 bb);
extern int onlyOne(U64 bb);

extern U64 getBishopAttacks(int square, U64 blockers);
extern U64 getRookAttacks(int square, U64 blockers);
extern U64 getAllPawnAttacks(U64 pawns, int side);
extern U64 getAllPawnDoubleAttacks(U64 pawns, int side);

extern U64 singlePawnPush(U64 pawns, U64 all_pieces, int color);

#endif
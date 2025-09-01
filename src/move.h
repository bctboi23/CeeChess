#ifndef MOVE_H
#define MOVE_H

#include "board.h"

/* GAME MOVE */
/* Current move: 32 bits. size is required for mailbox implementation */
/*
0000 0000 0000 0000 0000 0111 1111 -> From 0x7F
0000 0000 0000 0011 1111 1000 0000 -> To >> 7, 0x7F
0000 0000 0011 1100 0000 0000 0000 -> Captured >> 14, 0xF
0000 0000 0100 0000 0000 0000 0000 -> EP 0x40000
0000 0000 1000 0000 0000 0000 0000 -> Pawn Start 0x80000
0000 1111 0000 0000 0000 0000 0000 -> Promoted Piece >> 20, 0xF
0001 0000 0000 0000 0000 0000 0000 -> Castle 0x1000000
*/

/* New move: 16 bits. size can be compressed due to 8x8 representation instead of mailbox */
/*
0000 0000 0011 1111 -> From 0x7F
0000 1111 1100 0000 -> To >> 6, 0x7F
last four are (promo) (cap) (special 1) (special 0)
0000 -> Quiet move
0100 -> Capture
0001 -> Double pawn push
0010 -> Castle kingside
0011 -> Castle queenside
1000 -> Knight promo
1001 -> Bishop promo
1010 -> Rook promo
1011 -> Queen promo
1100 -> Knight promo cap
1101 -> Bishop promo cap
1110 -> Rook promo cap
1111 -> Queen promo cap

This saves space both in the move list and in the transposition table. Implement after changing everything to sq64
*/

#define MOVE(f,t,ca,pro,fl) ( (f) | ((t) << 7) | ( (ca) << 14 ) | ( (pro) << 20 ) | (fl))

#define FROMSQ(m) ((m) & 0x7F)
#define TOSQ(m) (((m)>>7) & 0x7F)
#define CAPTURED(m) (((m)>>14) & 0xF)
#define PROMOTED(m) (((m)>>20) & 0xF)

#define MFLAGEP 0x40000
#define MFLAGPS 0x80000
#define MFLAGCA 0x1000000

#define MFLAGCAP 0x7C000
#define MFLAGPROM 0xF00000

#define NOMOVE 0

#define HASH_PCE(pce,sq) (pos->posKey ^= (PieceKeys[(pce)][(sq)]))
#define HASH_CA (pos->posKey ^= (CastleKeys[(pos->castlePerm)]))
#define HASH_SIDE (pos->posKey ^= (SideKey))
#define HASH_EP (pos->posKey ^= (PieceKeys[EMPTY][(pos->enPas)]))

typedef struct {
	int move;
	int score;
} S_MOVE;

typedef struct {
	S_MOVE moves[MAXPOSITIONMOVES];
	int count;
} S_MOVELIST;

extern void MakeNullMove(S_BOARD *pos);
extern void TakeMove(S_BOARD *pos);
extern void TakeNullMove(S_BOARD *pos);

extern int MakeMove(S_BOARD *pos, int move);


#endif
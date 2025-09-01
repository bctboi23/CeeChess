// attack.c
#include "attack.h"
#include "bitboards.h"
#include "debug.h"

// checks if a square is attacked, so can exit early
int inline SqAttacked(const int sq, const int side, const S_BOARD *pos) {
	
	ASSERT(SqOnBoard(sq)); 
	ASSERT(SideValid(side));
	ASSERT(CheckBoard(pos));

	int side_idx = side * 6 - 1; // this indexes to the correct side in the bitboards in piece_bbs
	// pawns are weird because they matter by side
	if (pawnAttackTable[side ^ 1][sq] & pos->piece_bbs[side_idx + wP]) {
		return TRUE;
	}

	// knights 
	if (nonSliderMoveTable[0][sq] & pos->piece_bbs[side_idx + wN]) {
		return TRUE;
	}

	// rooks, queens
	if (getRookAttacks(sq, pos->color_bbs[BOTH]) & (pos->piece_bbs[side_idx + wR] | pos->piece_bbs[side_idx + wQ])) {
		return TRUE;
	}

	// bishops, queens
	if (getBishopAttacks(sq, pos->color_bbs[BOTH]) & (pos->piece_bbs[side_idx + wB] | pos->piece_bbs[side_idx + wQ])) {
		return TRUE;
	}

	// kings
	if (nonSliderMoveTable[1][sq] & pos->piece_bbs[side_idx + wK]) {
		return TRUE;
	}

	return FALSE;
}

// gets all pieces that attack a square, as a bitboard
U64 inline getAttacks(const int sq, const int side, const S_BOARD *pos) {
	
	ASSERT(SqOnBoard(sq)); 
	ASSERT(SideValid(side));
	ASSERT(CheckBoard(pos));

	int side_idx = side * 6 - 1; // this indexes to the correct side in the bitboards in piece_bbs
	return (pawnAttackTable[side ^ 1][sq] & pos->piece_bbs[side_idx + wP]) |
		   (nonSliderMoveTable[0][sq] & pos->piece_bbs[side_idx + wN]) |
		   (getRookAttacks(sq, pos->color_bbs[BOTH]) & (pos->piece_bbs[side_idx + wR] | pos->piece_bbs[side_idx + wQ])) |
		   (getBishopAttacks(sq, pos->color_bbs[BOTH]) & (pos->piece_bbs[side_idx + wB] | pos->piece_bbs[side_idx + wQ])) |
		   (nonSliderMoveTable[1][sq] & pos->piece_bbs[side_idx + wK]);
}
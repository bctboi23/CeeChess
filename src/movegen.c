// movegen.c

#include "attack.h"
#include "bitboards.h"
#include "movegen.h"
#include "debug.h"

// these are accessed by color, pce_idx
const int bishop_queen[2][2] = {
	{wB, wQ}, {bB, bQ}
};

const int rook_queen[2][2] = {
	{wR, wQ}, {bR, bQ}
};

const int non_sliders[2][2] = {
	{wN, wK}, {bN, bK}
};

const int PromotionRank[2] = { RANK_7, RANK_2 };
const U64 DoublePawnPushRank[2] = {0x00000000FF000000ULL, 0x000000FF00000000ULL};

/*
PV Move
Cap -> MvvLVA
Killers
HistoryScore

*/
const int VictimScore[13] = { 0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600 };
static int MvvLvaScores[13][13];

// we use a function pointer passed directly to the generic AddMove() in order to
// handle move scoring and avoid unnecessary branching and code duplication
typedef int (*MoveOrderFunction)(const S_BOARD *pos, int move);

void InitMvvLva() {
	int Attacker;
	int Victim;
	for(Attacker = wP; Attacker <= bK; ++Attacker) {
		for(Victim = wP; Victim <= bK; ++Victim) {
			MvvLvaScores[Victim][Attacker] = VictimScore[Victim] + 6 - ( VictimScore[Attacker] / 100);
		}
	}
}

int MoveExists(S_BOARD *pos, const int move) {

	S_MOVELIST list[1];
    GenerateAllMoves(pos,list);

    int MoveNum = 0;
	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

        if ( !MakeMove(pos,list->moves[MoveNum].move))  {
            continue;
        }
        TakeMove(pos);
		if(list->moves[MoveNum].move == move) {
			return TRUE;
		}
    }
	return FALSE;
}

static int scoreQuiet(const S_BOARD *pos, int move) {
	if(pos->searchKillers[0][pos->ply] == move) {
		return 900000;
	} 
	if (pos->searchKillers[1][pos->ply] == move) {
		return 800000;
	} 
	if (pos->searchKillers[0][pos->ply - 2] == move) {
		return 700000;
	} 
	if (pos->searchKillers[1][pos->ply - 2] == move) {
		return 600000;
	}
	return pos->searchHistory[pos->side][FROMSQ(move)][TOSQ(move)];
}

static int scoreCapture(const S_BOARD *pos, int move) {
	return 1000000 + MvvLvaScores[CAPTURED(move)][pos->pieces[FROMSQ(move)]];
}

static int scoreEnPassant(const S_BOARD *pos, int move) {
	return 1000000 + 105;
}

static int scoreQueenPromotion(const S_BOARD *pos, int move) {
	return 1000000 + 610 + VictimScore[CAPTURED(move)]; // this should score quiet queen promotions before captures
}

static void AddMove(const S_BOARD *pos, int move, S_MOVELIST *list, MoveOrderFunction moveOrder) {
	ASSERT(SqOnBoard(FROMSQ(move)));
	ASSERT(SqOnBoard(TOSQ(move)));
	ASSERT(CheckBoard(pos));
	ASSERT(pos->ply >=0 && pos->ply < MAXDEPTH);

	list->moves[list->count].move = move;
	list->moves[list->count].score = moveOrder(pos, move);
	list->count++;
	return;
}

// this handles pawn move promotions, since promotions create more moves
static void HandlePromotion(const S_BOARD *pos, const int from, const int to, const int cap, const int ep_sq, S_MOVELIST *list, MoveOrderFunction moveOrder) {
	ASSERT(SqOnBoard(FROMSQ(move)));
	ASSERT(SqOnBoard(TOSQ(move)));
	ASSERT(CheckBoard(pos));
	ASSERT(pos->ply >=0 && pos->ply < MAXDEPTH);

	if(RanksBrd[from] == PromotionRank[pos->side]) {
		int side_adj = 6 * pos->side;
		AddMove(pos, MOVE(from,to,cap,(side_adj + wQ),ep_sq), list, scoreQueenPromotion);
		AddMove(pos, MOVE(from,to,cap,(side_adj + wR),ep_sq), list, scoreQuiet);
		AddMove(pos, MOVE(from,to,cap,(side_adj + wB),ep_sq), list, scoreQuiet);
		AddMove(pos, MOVE(from,to,cap,(side_adj + wN),ep_sq), list, scoreQuiet);
	} else {
		AddMove(pos, MOVE(from,to,cap,EMPTY,ep_sq), list, moveOrder);
	}
	return;
}

void GenerateAllMoves(const S_BOARD *pos, S_MOVELIST *list) {

	ASSERT(CheckBoard(pos));

	list->count = 0;

	int pce = EMPTY;
	int side = pos->side;
	int opp_side = side ^ 1;

	int side_pawns = 6 * side + wP;
	// if we have a lookup table for possible pawn attacks on an ep square, we can generate all of the en passant attacks
	// at once via the pawn bitboard, without having to loop per piece. this is typically faster because the maximum possible ep attacks (2)
	// will most likely be lower than the number of pawns on the board on a given side (0-8)
	if (pos->enPas != NO_SQ) {
		U64 en_passant_bitboard = pawnAttackTable[opp_side][pos->enPas] & pos->piece_bbs[side_pawns - 1]; // TODO: generate en_passant_attacks -> en_passant_attacks[en_passant_square] & piece_bbs[6 * side];
		while (en_passant_bitboard) {
			int sq = POP_LSB(&en_passant_bitboard);
			AddMove(pos, MOVE(sq, pos->enPas, EMPTY, EMPTY, MFLAGEP), list, &scoreEnPassant); // since en passant promotion is impossible we skip checking for it
		}
	}
	// now, since pawn pushes never destructively interfere with each other, we can generate them all at once, to save time
	U64 single_pushes_bitboard = singlePawnPush(pos->piece_bbs[side_pawns - 1], pos->color_bbs[BOTH], side);
	U64 double_pushes_bitboard = singlePawnPush(single_pushes_bitboard, pos->color_bbs[BOTH], side) & DoublePawnPushRank[side];

	int direction = 8 - (16 * side);
	while (single_pushes_bitboard) {
		int t_sq = POP_LSB(&single_pushes_bitboard);
		HandlePromotion(pos, t_sq - direction, t_sq, EMPTY, EMPTY, list, &scoreQuiet);
	}
	while (double_pushes_bitboard) {
		int t_sq = POP_LSB(&double_pushes_bitboard);
		AddMove(pos, MOVE(t_sq - (2 * direction), t_sq, EMPTY, EMPTY, MFLAGPS), list, &scoreQuiet); // since double pawn push promotion is impossible we skip checking for it
	}

	U64 pawns = pos->piece_bbs[side_pawns - 1];
	for (int pceNum = 0; pceNum < pos->pceNum[side_pawns]; ++pceNum) {
		// Doing captures and quiets separate makes pawns easier, as they attack in a different pattern than they move
		int sq = POP_LSB(&pawns);
		U64 captures_bitboard = pawnAttackTable[side][sq] & pos->color_bbs[opp_side];
		while (captures_bitboard) {
			int t_sq = POP_LSB(&captures_bitboard);
			HandlePromotion(pos, sq, t_sq, pos->pieces[t_sq], 0, list, &scoreCapture);
		}
	}

	if(side == WHITE) {
		// TODO: Replace all (except castling moves) with bitboard lookups and pops, via the above code
		if(pos->castlePerm & WKCA) {
			if(pos->pieces[F1] == EMPTY && pos->pieces[G1] == EMPTY) {
				if(!SqAttacked(E1,BLACK,pos) && !SqAttacked(F1,BLACK,pos) ) {
					AddMove(pos, MOVE(E1, G1, EMPTY, EMPTY, MFLAGCA), list, &scoreQuiet);
				}
			}
		}

		if(pos->castlePerm & WQCA) {
			if(pos->pieces[D1] == EMPTY && pos->pieces[C1] == EMPTY && pos->pieces[B1] == EMPTY) {
				if(!SqAttacked(E1,BLACK,pos) && !SqAttacked(D1,BLACK,pos) ) {
					AddMove(pos, MOVE(E1, C1, EMPTY, EMPTY, MFLAGCA), list, &scoreQuiet);
				}
			}
		}

	} else {

		// castling
		if(pos->castlePerm &  BKCA) {
			if(pos->pieces[F8] == EMPTY && pos->pieces[G8] == EMPTY) {
				if(!SqAttacked(E8,WHITE,pos) && !SqAttacked(F8,WHITE,pos) ) {
					AddMove(pos, MOVE(E8, G8, EMPTY, EMPTY, MFLAGCA), list, &scoreQuiet);
				}
			}
		}

		if(pos->castlePerm &  BQCA) {
			if(pos->pieces[D8] == EMPTY && pos->pieces[C8] == EMPTY && pos->pieces[B8] == EMPTY) {
				if(!SqAttacked(E8,WHITE,pos) && !SqAttacked(D8,WHITE,pos) ) {
					AddMove(pos, MOVE(E8, C8, EMPTY, EMPTY, MFLAGCA), list, &scoreQuiet);
				}
			}
		}
	}

	/* Bishop queen moves */
	for (int i = 0; i < 2; i++) {
		pce = bishop_queen[side][i];
		ASSERT(PieceValid(pce));
		U64 piece_bb = pos->piece_bbs[pce - 1];
		for(int pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
			int sq = POP_LSB(&piece_bb);
			ASSERT(SqOnBoard(sq));
			U64 all_moves_bitboard = getBishopAttacks(sq, pos->color_bbs[BOTH]);
			// get all moves bitboard from lookup ANDed with NOT same color to remove moves to blockers
			// get captures from all moves AND with opposing color, non-captures from all moves AND with NOT opposing color
			// getting these separately allows for skipping an if statement in the loop
			U64 captures_bitboard = all_moves_bitboard & pos->color_bbs[opp_side];
			U64 quiets_bitboard = all_moves_bitboard & ~pos->color_bbs[BOTH];
			while (captures_bitboard) {
				int t_sq = POP_LSB(&captures_bitboard);
				AddMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list, &scoreCapture);
				
			}
			while (quiets_bitboard) {
				int t_sq = POP_LSB(&quiets_bitboard);
				AddMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list, &scoreQuiet);
			}
		}
	}

	/* Rook queen moves */
	for (int i = 0; i < 2; i++) {
		pce = rook_queen[side][i];
		ASSERT(PieceValid(pce));
		U64 piece_bb = pos->piece_bbs[pce - 1];
		for(int pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
			int sq = POP_LSB(&piece_bb);
			ASSERT(SqOnBoard(sq));
			U64 all_moves_bitboard = getRookAttacks(sq, pos->color_bbs[BOTH]);
			// get all moves bitboard from lookup ANDed with NOT same color to remove moves to blockers
			// get captures from all moves AND with opposing color, non-captures from all moves AND with NOT opposing color
			// getting these separately allows for skipping an if statement in the loop
			U64 captures_bitboard = all_moves_bitboard & pos->color_bbs[opp_side];
			U64 quiets_bitboard = all_moves_bitboard & ~pos->color_bbs[BOTH];
			while (captures_bitboard) {
				int t_sq = POP_LSB(&captures_bitboard);
				AddMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list, &scoreCapture);
				
			}
			while (quiets_bitboard) {
				int t_sq = POP_LSB(&quiets_bitboard);
				AddMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list, &scoreQuiet);
			}
		}
	}

	/* Non slider moves */
	for (int i = 0; i < 2; i++) {
		pce = non_sliders[side][i];
		ASSERT(PieceValid(pce));
		U64 piece_bb = pos->piece_bbs[pce - 1];
		for(int pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {	
			int sq = POP_LSB(&piece_bb);
			ASSERT(SqOnBoard(sq));
			// get all moves bitboard from lookup ANDed with NOT same color to remove moves to blockers
			// get captures from all moves AND with opposing color, non-captures from all moves AND with NOT opposing color
			// getting these separately allows for skipping an if statement in the loop
			U64 captures_bitboard = nonSliderMoveTable[i][sq] & pos->color_bbs[opp_side];
			U64 quiets_bitboard = nonSliderMoveTable[i][sq] & ~pos->color_bbs[BOTH];
			while (captures_bitboard) {
				int t_sq = POP_LSB(&captures_bitboard);
				AddMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list, &scoreCapture);
				
			}
			while (quiets_bitboard) {
				int t_sq = POP_LSB(&quiets_bitboard);
				AddMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list, &scoreQuiet);
			}
		}
	}

    ASSERT(MoveListOk(list,pos));
}


void GenerateAllCaps(const S_BOARD *pos, S_MOVELIST *list) {

	ASSERT(CheckBoard(pos));

	list->count = 0;

	int pce = EMPTY;
	int side = pos->side;
	int opp_side = side ^ 1;

	int side_pawns = 6 * side + wP;
	if (pos->enPas != NO_SQ) {
		U64 en_passant_bitboard = pawnAttackTable[opp_side][pos->enPas] & pos->piece_bbs[side_pawns - 1]; // TODO: generate en_passant_attacks -> en_passant_attacks[en_passant_square] & piece_bbs[6 * side];
		while (en_passant_bitboard) {
			int sq = POP_LSB(&en_passant_bitboard);
			AddMove(pos, MOVE(sq, pos->enPas, EMPTY, EMPTY, MFLAGEP), list, &scoreEnPassant); // since en passant promotion is impossible we skip checking for it
		}
	}

	// include quiet promotions in capture generation (for QS)
	int direction = 8 - (16 * side);
	U64 promotion_mask = RankBBMask[PromotionRank[side]];
	U64 promotion_bitboard = singlePawnPush(pos->piece_bbs[side_pawns - 1] & promotion_mask, pos->color_bbs[BOTH], side);
	while (promotion_bitboard) {
		int t_sq = POP_LSB(&promotion_bitboard);
		HandlePromotion(pos, t_sq - direction, t_sq, EMPTY, EMPTY, list, &scoreQuiet);
	}

	U64 pawns = pos->piece_bbs[side_pawns - 1];
	for (int pceNum = 0; pceNum < pos->pceNum[side_pawns]; ++pceNum) {
		// Doing captures and quiets separate makes pawns easier, as they attack in a different pattern than they move
		int sq = POP_LSB(&pawns);
		U64 captures_bitboard = pawnAttackTable[side][sq] & pos->color_bbs[opp_side];
		while (captures_bitboard) {
			int t_sq = POP_LSB(&captures_bitboard);
			HandlePromotion(pos, sq, t_sq, pos->pieces[t_sq], 0, list, &scoreCapture);
		}
	}

	/* Bishop queen moves */
	for (int i = 0; i < 2; i++) {
		pce = bishop_queen[side][i];
		ASSERT(PieceValid(pce));
		U64 piece_bb = pos->piece_bbs[pce - 1];
		for(int pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
			int sq = POP_LSB(&piece_bb);
			ASSERT(SqOnBoard(sq));
			// get captures from all moves AND with opposing color
			U64 captures_bitboard = getBishopAttacks(sq, pos->color_bbs[BOTH]) & pos->color_bbs[opp_side];
			while (captures_bitboard) {
				int t_sq = POP_LSB(&captures_bitboard);
				AddMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list, &scoreCapture);
				
			}
		}
	}

	/* Rook queen moves */
	for (int i = 0; i < 2; i++) {
		pce = rook_queen[side][i];
		ASSERT(PieceValid(pce));
		U64 piece_bb = pos->piece_bbs[pce - 1];
		for(int pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
			int sq = POP_LSB(&piece_bb);
			ASSERT(SqOnBoard(sq));
			// get captures from all moves AND with opposing color
			U64 captures_bitboard = getRookAttacks(sq, pos->color_bbs[BOTH]) & pos->color_bbs[opp_side];
			while (captures_bitboard) {
				int t_sq = POP_LSB(&captures_bitboard);
				AddMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list, &scoreCapture);
				
			}
		}
	}

	/* Non slider moves */
	for (int i = 0; i < 2; i++) {
		pce = non_sliders[side][i];
		ASSERT(PieceValid(pce));
		U64 piece_bb = pos->piece_bbs[pce - 1];
		for(int pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {	
			int sq = POP_LSB(&piece_bb);
			ASSERT(SqOnBoard(sq));
			// get captures from all moves AND with opposing color
			U64 captures_bitboard = nonSliderMoveTable[i][sq] & pos->color_bbs[opp_side];
			while (captures_bitboard) {
				int t_sq = POP_LSB(&captures_bitboard);
				AddMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list, &scoreCapture);
				
			}
		}
	}

    ASSERT(MoveListOk(list,pos));
}
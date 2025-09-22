// evaluate.c

#include "evaluate.h"
#include "bitboards.h"
#include "move.h"
#include "eval-tuned.h"
#include "debug.h"
#include "attack.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct  {
	U64 piece_attacks[12];
	U64 color_attacks[2];
	U64 attacked_by_two[2];
	U64 king_zone[2];
	U64 mobility_sqs[2];
	U64 rammed_pawns[2];
	int numAttackers[2];
	int attackScore[2];
} S_EVAL_INFO;

// tempo is kept at a constant but small value
const int tempo = 3;

int DistTable[64][64];

// for use in detecting same color bishops
U64 white_sqs = 0x55aa55aa55aa55aa;
U64 black_sqs = 0xaa55aa55aa55aa55;

// for use in detecting center control
U64 center_sqs = 0x1818000000;

// for use in scale evaluation
U64 left_flank = 0xf0f0f0f0f0f0f0f;
U64 right_flank = 0xf0f0f0f0f0f0f0f0;

// for use in mirrored file evaluation
int mirrored_file[8] = {0, 1, 2, 3, 3, 2, 1, 0};

void InitEval() {
	for (int i = 0; i < 64; ++i) {
		for (int j = 0; j < 64; ++j) {
			// distance is max of file-wise or row-wise distance
			DistTable[i][j] = MAX(abs(COL(i) - COL(j)), abs(ROW(i) - ROW(j)));
      	}
   	}
}

static int MaterialDraw(const S_BOARD *pos) {

	if (pos->pceNum[wP] || pos->pceNum[bP]) {
		return FALSE;
	}

	ASSERT(CheckBoard(pos));

	if (!pos->pceNum[wR] && !pos->pceNum[bR] && !pos->pceNum[wQ] && !pos->pceNum[bQ]) {
		
		if (!pos->pceNum[bB] && !pos->pceNum[wB]) {

			if (pos->pceNum[wN] < 3 && pos->pceNum[bN] < 3)
				return TRUE;

		} else if (!pos->pceNum[wN] && !pos->pceNum[bN]) {

			if (abs(pos->pceNum[wB] - pos->pceNum[bB]) < 2)
				return TRUE;

			// if all bishops are on the same color it is a draw
			U64 bishops_bb = (pos->piece_bbs[wB - 1] | pos->piece_bbs[bB - 1]);
			if (!(bishops_bb & white_sqs) || !(bishops_bb & black_sqs))
				return TRUE;

		} else if ((pos->pceNum[wN] < 3 && !pos->pceNum[wB]) || (pos->pceNum[wB] == 1 && !pos->pceNum[wN])) {

			if ((pos->pceNum[bN] < 3 && !pos->pceNum[bB]) || (pos->pceNum[bB] == 1 && !pos->pceNum[bN]))
				return TRUE;
		}
	} else if (!pos->pceNum[wQ] && !pos->pceNum[bQ]) {

		if (pos->pceNum[wR] == 1 && pos->pceNum[bR] == 1) {

			if ((pos->pceNum[wN] + pos->pceNum[wB]) < 2 && (pos->pceNum[bN] + pos->pceNum[bB]) < 2)
				return TRUE;

		} else if (pos->pceNum[wR] == 1 && !pos->pceNum[bR]) {

			if ((pos->pceNum[wN] + pos->pceNum[wB] == 0) && (((pos->pceNum[bN] + pos->pceNum[bB]) == 1) || ((pos->pceNum[bN] + pos->pceNum[bB]) == 2)))
				 return TRUE;
		} else if (pos->pceNum[bR] == 1 && !pos->pceNum[wR]) {

			if ((pos->pceNum[bN] + pos->pceNum[bB] == 0) && (((pos->pceNum[wN] + pos->pceNum[wB]) == 1) || ((pos->pceNum[wN] + pos->pceNum[wB]) == 2))) 
				return TRUE;
		}
	}
	return FALSE;
}

int evalKnights(const S_BOARD *pos, S_EVAL_INFO *info, const S_EVAL_PARAMS *params, const int color) {

	int us = color;
	int them = !color;

	int score = 0;
	int pce = us * 6 + wN;
	int our_color_idx = us * 6 - 1;
	int enemy_color_idx = them * 6 - 1;

	U64 curr_pieces = pos->piece_bbs[pce - 1];

	U64 enemy_pawns = pos->piece_bbs[enemy_color_idx + wP];
	U64 both_pawns = (pos->piece_bbs[wP - 1] | pos->piece_bbs[bP - 1]);

	U64 OutpostRanks = (us == WHITE ? RankBBMask[RANK_4] | RankBBMask[RANK_5] | RankBBMask[RANK_6]
                                    : RankBBMask[RANK_5] | RankBBMask[RANK_4] | RankBBMask[RANK_3]);

	// potential outposts are squares on the outpost ranks that are defended by our pawns
	U64 poentialOutposts = OutpostRanks & info->piece_attacks[our_color_idx + wP];

	for(int i = 0; i < pos->pceNum[pce]; i++) {
		int sq = POP_LSB(&curr_pieces);

		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);

		score += params->KnightPSQT[PSQT_SQ(sq)];

		// mobility
		U64 attacks = nonSliderMoveTable[0][sq];
		int move_num = POP_CNT(attacks & info->mobility_sqs[us]); 
		score += params->KnightMobility[move_num];

		// king safety
		int king_zone_attacks = POP_CNT(attacks & info->king_zone[them]);
		if (king_zone_attacks > 0) {
			info->attackScore[us] += params->KnightAttack * king_zone_attacks + params->KnightAttacker;
			info->numAttackers[us]++;
		}

		// add a bonus for a knight outpost
		if (CHKBIT(poentialOutposts, sq) && ~(getPawnAttackSpan(sq, us) & enemy_pawns))
			score += params->KnightOutpost;

		// add a bonus for knights behind a pawn
		if (CHKBIT(singlePawnPush(both_pawns, 0ull, them), sq))
			score += params->KnightBehindPawn;

		// add a malus for knights in siberia (away from both kings)
		int kingDist = MIN(DistTable[pos->KingSq[us]][sq], DistTable[pos->KingSq[them]][sq]);
		if (kingDist >= 4)
			score += params->KnightInSiberia[kingDist - 4];

		// update attacks
		info->attacked_by_two[us] |= (attacks & info->color_attacks[us]);
		info->color_attacks[us] |= attacks;
		info->piece_attacks[pce - 1] |= attacks;
	}

	return score;
}

int evalBishops(const S_BOARD *pos, S_EVAL_INFO *info, const S_EVAL_PARAMS *params, const int color) {

	int us = color;
	int them = !color;

	int score = 0;
	int pce = us * 6 + wB;
	U64 curr_pieces = pos->piece_bbs[pce - 1];

	int our_color_idx = us * 6 - 1;
	int enemy_color_idx = them * 6 - 1;

	U64 both_pawns = (pos->piece_bbs[our_color_idx + wP] | pos->piece_bbs[enemy_color_idx + wP]);
	U64 both_queens = (pos->piece_bbs[our_color_idx + wQ] | pos->piece_bbs[enemy_color_idx + wQ]);

	// for mobility and king safety calculations, bishops and rooks xray through themselves and queens
	U64 bishop_blockers = pos->color_bbs[BOTH] ^ (curr_pieces | both_queens);

	// bishop pair (only counts with opposite color bishops)
	if ((curr_pieces & white_sqs) && (curr_pieces & black_sqs))
		score += params->BishopPair;

	for(int i = 0; i < pos->pceNum[pce]; i++) {
		int sq = POP_LSB(&curr_pieces);

		ASSERT(SqOnBoard(sq)); 
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);

		// psqt
		score += params->BishopPSQT[PSQT_SQ(sq)];

		// mobility
		U64 attacks = getBishopAttacks(sq, bishop_blockers);
		int move_num = POP_CNT(attacks & info->mobility_sqs[us]);
		score += params->BishopMobility[move_num];

		// king safety
		int king_zone_attacks = POP_CNT(attacks & info->king_zone[them]);
		if (king_zone_attacks > 0) {
			info->attackScore[us] +=  params->BishopAttack * king_zone_attacks + params->BishopAttacker;
			info->numAttackers[us]++;
		}

		// add a bonus for bishops behind a pawn
		if (CHKBIT(singlePawnPush(both_pawns, 0ull, them), sq))
			score += params->BishopBehindPawn;

		// add a bonus for bishops that attack both center squares on the long diagonal
		if (CHKBIT(~center_sqs, sq) && several(attacks & center_sqs))
			score += params->BishopLongDiagonal;
		
		// add a malus for bishops on the same color squares as any rammed pawns
		U64 matching_color_sqs = CHKBIT(white_sqs, sq) ? white_sqs : black_sqs;
		score += params->BishopRammedPawns * POP_CNT(info->rammed_pawns[us] & matching_color_sqs);

		// update attacks
		info->attacked_by_two[us] |= (attacks & info->color_attacks[us]);
		info->color_attacks[us] |= attacks;
		info->piece_attacks[pce - 1] |= attacks;
	}

	return score;
}

int evalRooks(const S_BOARD *pos, S_EVAL_INFO *info, const S_EVAL_PARAMS *params, const int color) {

	int us = color;
	int them = !color;

	int score = 0;
	int pce = us * 6 + wR;
	U64 curr_pieces = pos->piece_bbs[pce - 1];

	int our_color_idx = us * 6 - 1;
	int enemy_color_idx = them * 6 - 1;

	U64 both_pawns = (pos->piece_bbs[our_color_idx + wP] | pos->piece_bbs[enemy_color_idx + wP]);
	U64 both_queens = (pos->piece_bbs[our_color_idx + wQ] | pos->piece_bbs[enemy_color_idx + wQ]);

	// for mobility and king safety calculations, bishops and rooks xray through themselves and queens
	U64 rook_blockers = pos->color_bbs[BOTH] ^ (curr_pieces | both_queens);

	for(int i = 0; i < pos->pceNum[pce]; i++) {
		int sq = POP_LSB(&curr_pieces);

		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);

		score += params->RookPSQT[PSQT_SQ(sq)];

		U64 rook_file_mask = FileBBMask[COL(sq)];

		// mobility
		U64 attacks = getRookAttacks(sq, rook_blockers);
		int move_num = POP_CNT(attacks & info->mobility_sqs[us]);
		score += params->RookMobility[move_num];

		// king safety
		int king_zone_attacks = POP_CNT(attacks & info->king_zone[them]);
		if (king_zone_attacks > 0) {
			info->attackScore[us] += params->RookAttack * king_zone_attacks + params->RookAttacker;
			info->numAttackers[us]++;
		}

		// Rook open and semi open file bonuses
		if(!(pos->piece_bbs[our_color_idx + wP] & rook_file_mask)) {
			int open = !(both_pawns & FileBBMask[COL(sq)]);
			score += params->RookFile[open];
		}

		// bonus for rook on 7th rank and attacking enemy pawns or cutting off enemy king
		if (RELATIVE_ROW(sq, us) == RANK_7) {
			if ((pos->piece_bbs[enemy_color_idx + wP] & RankBBMask[ROW(sq)]) | 
				(RELATIVE_ROW(pos->KingSq[them], us) >= RANK_7)) {
				score += params->RookOn7th;
			}
		}

		// bonus for rook on a queen file
        if (rook_file_mask & both_queens)
            score += params->RookOnQueenFile;

		// update attacks
		info->attacked_by_two[us] |= (attacks & info->color_attacks[us]);
		info->color_attacks[us] |= attacks;
		info->piece_attacks[pce - 1] |= attacks;
	}

	return score;
}

int evalQueens(const S_BOARD *pos, S_EVAL_INFO *info, const S_EVAL_PARAMS *params, const int color) {

	int us = color;
	int them = !color;

	int score = 0;
	int pce = us * 6 + wQ;
	int them_color_idx = them * 6 - 1;
	U64 curr_pieces = pos->piece_bbs[pce - 1];

	// for mobility calculations queens get no bonus for squares the enemy attacks with minor pieces
	U64 minor_piece_attacks = (info->piece_attacks[them_color_idx + wN] | info->piece_attacks[them_color_idx + wB]);

	for(int i = 0; i < pos->pceNum[pce]; i++) {
		int sq = POP_LSB(&curr_pieces);

		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);

		score += params->QueenPSQT[PSQT_SQ(sq)];

		// mobility
		U64 attacks = getBishopAttacks(sq, pos->color_bbs[BOTH]) | getRookAttacks(sq, pos->color_bbs[BOTH]);
		int move_num = POP_CNT(attacks & info->mobility_sqs[us] & ~minor_piece_attacks);
		score += params->QueenMobility[move_num];

		// king safety
		int king_zone_attacks = POP_CNT(attacks & info->king_zone[them]);
		if (king_zone_attacks > 0) {
			info->attackScore[us] += params->QueenAttack * king_zone_attacks + params->QueenAttacker;
			info->numAttackers[us]++;
		}
		
		// update attacks
		info->attacked_by_two[us] |= (attacks & info->color_attacks[us]);
		info->color_attacks[us] |= attacks;
		info->piece_attacks[pce - 1] |= attacks;
	}

	return score;
}

int evalKings(const S_BOARD *pos, S_EVAL_INFO *info, const S_EVAL_PARAMS *params, const int color) {

	int us = color;
	int them = !color;

	int us_color_idx = us * 6 - 1;
	int them_color_idx = them * 6 - 1;

	int our_king_sq = pos->KingSq[us];

	int score = params->KingPSQT[PSQT_SQ(our_king_sq)];

	// apply king safety if the enemy has enough attackers
	// structure from Ethereal
	if (info->numAttackers[them] > 1) {

		// get the initial score from the attacks
		int safetyScore = info->attackScore[them];

		// if the enemy has no queen, reduce their attack score (attacks are weaker when you can't bring in the queen)
		safetyScore += params->NoQueen * !(pos->piece_bbs[them_color_idx + wQ]);

		// if we have same color defenders in the king area (without double pawn attacks removed), decrease the attack score for the enemy
		U64 defenders = pos->piece_bbs[us_color_idx + wP] | 
						pos->piece_bbs[us_color_idx + wN] | 
						pos->piece_bbs[us_color_idx + wB];
		safetyScore += params->MinorDefenders * POP_CNT(defenders & KingAreaMasks[us][pos->KingSq[us]]);

		// if our pawn shield is intact, decrease the enemy attack score
		// we only give a bonus for a full pawn shield, as the MinorDefenders term accounts for defending pawns
		U64 currPawnShield = (pos->piece_bbs[us_color_idx + wP] & PawnShieldMasks[us][pos->KingSq[us]]);
		safetyScore += params->PawnShield * (currPawnShield == PawnShieldMasks[us][pos->KingSq[us]]);

		// if enemy pawns are close to (one push away) or in the king area, increase the enemy attack (pawn storm)
		// if a pawn push would get them out of the king area, the pawn isn't affecting the attack, and thus is not counted
		U64 currPawnStorm = singlePawnPush(pos->piece_bbs[them_color_idx + wP], 0ULL, them) & info->king_zone[us];
		safetyScore += params->PawnStorm * POP_CNT(currPawnStorm);

		// weak squares are attacked by them, defended no more than once and only defended by our Queens or King
        U64 weak = info->color_attacks[them] & 
					~info->attacked_by_two[us] & 
		(~info->color_attacks[us] | info->piece_attacks[us_color_idx + wQ] | info->piece_attacks[us_color_idx + wK]);
		safetyScore += params->WeakSquare * POP_CNT(weak & info->king_zone[us]);

		// Safe target squares are not defended by us or are weak and attacked twice by them
        // We exclude squares containing pieces which they cannot capture
		U64 safe = ~pos->color_bbs[them] & (~info->color_attacks[us] | (weak & info->attacked_by_two[them]));

		// find all safe checks, and increase the enemy attack by the counts
		U64 knightThreats = nonSliderMoveTable[0][our_king_sq] & info->piece_attacks[them_color_idx + wN];
        U64 bishopThreats = getBishopAttacks(our_king_sq, pos->color_bbs[BOTH]) & info->piece_attacks[them_color_idx + wB];
        U64 rookThreats   = getRookAttacks(our_king_sq, pos->color_bbs[BOTH]) & info->piece_attacks[them_color_idx + wR];
        U64 queenThreats  = (bishopThreats | rookThreats) & info->piece_attacks[them_color_idx + wQ];
		safetyScore += params->KnightCheck * POP_CNT(knightThreats & safe) + 
					   params->BishopCheck * POP_CNT(bishopThreats & safe) +
					   params->RookCheck   * POP_CNT(rookThreats   & safe) +
					   params->QueenCheck  * POP_CNT(queenThreats  & safe);

		// adjust the attack threshold by the number of pieces attacking (up to 5 pieces)
		// there has to be at least 2 attackers for the king safety code to run so we condense the range
		int attackerAdjIdx = MIN(info->numAttackers[them], 5) - 2;
		safetyScore += params->attackerAdj[attackerAdjIdx];

		// finally reduce our score by the king safety score
		// we cap the safety score at zero, and square it in the midgame
		// this gives small attacks less power, but big attacks even more power
		int midgame_safety_score = (uint32_t) MAX(0, SCORE_MG(safetyScore));
		int endgame_safety_score = (uint32_t) MAX(0, SCORE_EG(safetyScore));
		score -= MAKE_SCORE(midgame_safety_score * midgame_safety_score / 4096, 
							endgame_safety_score / 16); 
	}

	return score;

}

int evalPawns(const S_BOARD *pos, S_EVAL_INFO *info, const S_EVAL_PARAMS *params, const int color) {

	int us = color;
	int them = !color;

	int score = 0;
	int pce = us * 6 + wP;
	U64 curr_pieces = pos->piece_bbs[pce - 1];

	int our_pawn_idx = us * 6 + wP - 1;
	int enemy_pawn_idx = them * 6 + wP - 1;

	for(int i = 0; i < pos->pceNum[pce]; i++) {
		int sq = POP_LSB(&curr_pieces);

		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);

		int pawn_relative_row = RELATIVE_ROW(sq, us);
		int pawn_file = COL(sq);
		int pawn_row = ROW(sq);

		U64 pawn_push_no_blockers = singlePawnPush(1ULL << sq, 0ULL, us);
		U64 stoppers = PassedMask[us][sq] & pos->piece_bbs[enemy_pawn_idx];
		U64 levers = pos->piece_bbs[enemy_pawn_idx] & pawnAttackTable[us][sq];

		// psqt score
		score += params->PawnPSQT[PSQT_SQ(sq)];

		// isolated pawns
		// we apply the bonus by file, because some files with isolated pawns hurt worse than others
		if( (IsolatedMask[sq] & pos->piece_bbs[our_pawn_idx]) == 0) {
			//printf("wP Iso:%s\n",PrSq(sq));
			score += params->PawnIsolated[mirrored_file[pawn_file]];
		}
		// doubled pawns (we don't count a pawn as doubled if it can capture to leave the double) 
		// unlike the typical definition, we only apply the penalty when pawn is directly infront of another friendly pawn
		// we apply the bonus by file, because some files with doubled pawns hurt worse than others
		if ((pawn_push_no_blockers & pos->piece_bbs[our_pawn_idx]) && !(pawnAttackTable[us][sq] & pos->color_bbs[them])) {
			score += params->PawnDoubled[mirrored_file[pawn_file]];
		}
		// connected pawns
		if ((ConnectedMask[us][sq] & pos->piece_bbs[our_pawn_idx]) != 0) {
			score += params->PawnConnected;
		}

		// passed pawns, if pawn is advanced enough
		if(!(stoppers ^ levers) && (pawn_relative_row > RANK_3)) {
			//printf("wP Passed:%s\n",PrSq(sq))
			
			// get the base passer score
			score += params->PassedRank[pawn_relative_row];

			// add a bonus if the pawn can currently advance (no pieces)
			int pawnCanAdvance = !(pos->color_bbs[BOTH] & pawn_push_no_blockers);
			score += params->PawnCanAdvance[pawn_relative_row] * pawnCanAdvance;

			// add a bonus if pawn can safely advance (no pieces or enemy attacks)
			int safeAdvance = !((pos->color_bbs[BOTH] | info->color_attacks[them]) & pawn_push_no_blockers);
			score += params->PawnSafeAdvance[pawn_relative_row] * safeAdvance;

			// add a malus if the pawn is currently leverable
			score += params->PassedLeverable * (stoppers != 0);
						
			// add some more bonuses and maluses for the most advanced pawn on the file
			U64 pawn_path = ForwardRanksMasks[us][pawn_row] & FileBBMask[pawn_file];
			if (!(pawn_path & pos->piece_bbs[our_pawn_idx])) {
				
				// add a bonus if pawn's promotion path is uncontested by the enemy and not behind our pawns
				int uncontested = !(pawn_path & (info->color_attacks[them] | pos->color_bbs[them]));
				score += uncontested * params->SafePromotionPath;

				// add a bonus and a malus based on own king distance and enemy king distance to pawn square
				score += params->OwnKingPawnTropism * DistTable[sq][pos->KingSq[us]];
				score += params->EnemyKingPawnTropism * DistTable[sq][pos->KingSq[them]];
			}
		}
	} 

	return score;
}

// evaluating threats in the position (the Ethereal way)
int evalThreats(const S_BOARD *pos, const S_EVAL_INFO *info, const S_EVAL_PARAMS *params, const int color) {

	int us = color;
	int them = !color;

	int us_color_idx = us * 6 - 1;
	int them_color_idx = them * 6 - 1;

	int score = 0;
	int count = 0;

	U64 our_pawns  = pos->piece_bbs[us_color_idx + wP];
	U64 our_minors = pos->piece_bbs[us_color_idx + wN] | pos->piece_bbs[us_color_idx + wB];
	U64 our_rooks  = pos->piece_bbs[us_color_idx + wR];
	U64 our_queens = pos->piece_bbs[us_color_idx + wQ];

	U64 their_pawn_attacks  = info->piece_attacks[them_color_idx + wP];
	U64 their_minor_attacks = info->piece_attacks[them_color_idx + wN] | info->piece_attacks[them_color_idx + wB];
	U64 their_major_attacks = info->piece_attacks[them_color_idx + wR] | info->piece_attacks[them_color_idx + wQ];
	U64 their_king_attacks  = info->piece_attacks[them_color_idx + wK];

	U64 their_less_than_queen_attacks = their_pawn_attacks | their_minor_attacks | info->piece_attacks[them_color_idx + wR];

	// Squares with more attackers, few defenders, and no pawn support are considered poorly defended
    U64 poorlyDefended = (info->color_attacks[them] & ~info->color_attacks[us]) |
                         (info->attacked_by_two[them] & ~info->attacked_by_two[us] & ~info->piece_attacks[us_color_idx + wP]);
	
	U64 weakMinors = our_minors & poorlyDefended;

	// malus for any threat against weak pawns
	count = POP_CNT(our_pawns & poorlyDefended);
	score += count * params->WeakPawn;

	// malus for any pawn threat against minor pieces
	count = POP_CNT(our_minors & their_pawn_attacks);
	score += count * params->MinorAttackedByPawn;
 
	// malus for any minor threat against minor pieces
	count = POP_CNT(our_minors & their_minor_attacks);
	score += count * params->MinorAttackedByMinor;

	// malus for all major threats against poorly defended minors
	count = POP_CNT(weakMinors & their_major_attacks);
	score += count * params->MinorAttackedByMajor;

	// malus for pawn and minor threats against our rooks
	count = POP_CNT(our_rooks & (their_pawn_attacks | their_minor_attacks));
	score += count * params->RookAttackedByLesser;

	// malus for king threats against our poorly defended minors
	count = POP_CNT(weakMinors & their_king_attacks);
	score += count * params->MinorAttackedByKing;

	// malus for king threats against our undefended rooks
	count = POP_CNT((~info->color_attacks[us] & our_rooks) & their_king_attacks);
	score += count * params->RookAttackedByKing;

	// malus for any queens attacked by lesser pieces
	count = POP_CNT(our_queens & their_less_than_queen_attacks); 
	score += count * params->QueenAttackedByPiece;

	// malus for enemy restricting our piece moves (our pieces can't move to poorly defended squares)
	count = POP_CNT(info->color_attacks[us] & poorlyDefended);
    score += params->RestrictedPiece * count;

	return score; 
}

// from a given position and evaluation, get endgame scale based on other endgame factors, again from Ethereal (Looked at Stockfish eval guide as well)
int evalScale(const S_BOARD *pos, const int eval) {
	int eg_score = SCORE_EG(eval);
	int winning_side = (eg_score < 0); // (black is 1, white is 0)
	int winning_side_idx = 6 * winning_side;
	int losing_side_idx = 6 * (!winning_side);

	U64 bishops = pos->piece_bbs[wB - 1] | pos->piece_bbs[bB - 1];
	U64 queens = pos->piece_bbs[wQ - 1] | pos->piece_bbs[bQ - 1];
	U64 pawns = pos->piece_bbs[wP - 1] | pos->piece_bbs[bP - 1];
	U64 kings = pos->piece_bbs[wK - 1] | pos->piece_bbs[bK - 1];
	U64 non_pieces = pawns | kings;

	U64 non_queen_pieces = pos->color_bbs[BOTH] ^ (non_pieces | queens);
	U64 non_bishop_pieces = pos->color_bbs[BOTH] ^ (non_pieces | bishops);

	int pawn_advantage = pos->pceNum[winning_side_idx + wP] - pos->pceNum[losing_side_idx + wP];
	
	// opposite colored bishops endgames are hard to win
	if (!non_bishop_pieces && onlyOne(pos->piece_bbs[wB - 1]) && onlyOne(pos->piece_bbs[bB - 1]) && onlyOne(bishops & white_sqs))
		return SCALE_OCB;

	// lone minor vs king + pawns is never a win
	U64 winning_side_minors = pos->piece_bbs[winning_side_idx + wN - 1] | pos->piece_bbs[winning_side_idx + wB - 1];
    if ((winning_side_minors) && POP_CNT(pos->color_bbs[winning_side]) == 2)
        return 0;

	// lone queens are weak against multiple non-queen pieces
    if (onlyOne(queens) && several(non_queen_pieces) && non_queen_pieces == (pos->color_bbs[!winning_side] & non_queen_pieces))
		return SCALE_LONE_QUEEN;

	// scale up lone pieces with large pawn advantages
	if (!queens && !several(non_queen_pieces & pos->color_bbs[WHITE]) && !several(non_queen_pieces & pos->color_bbs[BLACK]) && pawn_advantage > 2)
        return SCALE_LARGE_PAWN_ADV;

	// scale down slightly if pawns aren't on both flanks
	if (!((pawns & left_flank) && (pawns & right_flank)))
		return SCALE_ONE_FLANK;

	return SCALE_NORMAL;
}



int EvalPosition(const S_BOARD *pos, const S_EVAL_PARAMS *params) {

	ASSERT(CheckBoard(pos));

	if (MaterialDraw(pos))
		return 0;

	// set up eval info
	S_EVAL_INFO eval_info[1] = {0};

	// add pawn attacks into eval info before computing all pieces (mobility reasons)
	eval_info->piece_attacks[wP - 1] = getAllPawnAttacks(pos->piece_bbs[wP - 1], WHITE);
	eval_info->piece_attacks[bP - 1] = getAllPawnAttacks(pos->piece_bbs[bP - 1], BLACK);
	eval_info->color_attacks[WHITE] = eval_info->piece_attacks[wP - 1];
	eval_info->color_attacks[BLACK] = eval_info->piece_attacks[bP - 1];
	eval_info->attacked_by_two[WHITE] = getAllPawnDoubleAttacks(pos->piece_bbs[wP - 1], WHITE);
	eval_info->attacked_by_two[BLACK] = getAllPawnDoubleAttacks(pos->piece_bbs[bP - 1], BLACK);

	// add rammed pawns for bishop evaluation
	eval_info->rammed_pawns[WHITE] = singlePawnPush(pos->piece_bbs[bP - 1], ~pos->piece_bbs[wP - 1], BLACK);
    eval_info->rammed_pawns[BLACK] = singlePawnPush(pos->piece_bbs[wP - 1], ~pos->piece_bbs[bP - 1], WHITE);

	// we remove pawn attacks and squares we cannot capture from valid mobility squares
	eval_info->mobility_sqs[WHITE] = ~eval_info->piece_attacks[bP - 1] & ~pos->color_bbs[WHITE];
	eval_info->mobility_sqs[BLACK] = ~eval_info->piece_attacks[wP - 1] & ~pos->color_bbs[BLACK];

	// we remove squares attacked twice by pawns from the king attack zone
	eval_info->king_zone[WHITE] = KingAreaMasks[WHITE][pos->KingSq[WHITE]] & ~eval_info->attacked_by_two[WHITE];
	eval_info->king_zone[BLACK] = KingAreaMasks[BLACK][pos->KingSq[BLACK]] & ~eval_info->attacked_by_two[BLACK];
	
	// pawn attacks count for king safety, but don't count for number of attackers
	eval_info->attackScore[WHITE] += params->PawnAttack * POP_CNT(eval_info->piece_attacks[wP - 1] & eval_info->king_zone[WHITE]);
	eval_info->attackScore[BLACK] += params->PawnAttack * POP_CNT(eval_info->piece_attacks[bP - 1] & eval_info->king_zone[BLACK]);

	// add king attacks too, they don't count for king safety
	U64 attacks = nonSliderMoveTable[1][pos->KingSq[WHITE]];
	eval_info->attacked_by_two[WHITE] |= (attacks & eval_info->color_attacks[WHITE]);
	eval_info->color_attacks[WHITE] |= attacks;
	eval_info->piece_attacks[wK - 1] |= attacks;

	attacks = nonSliderMoveTable[1][pos->KingSq[BLACK]];
	eval_info->attacked_by_two[BLACK] |= (attacks & eval_info->color_attacks[BLACK]);
	eval_info->color_attacks[BLACK] |= attacks;
	eval_info->piece_attacks[bK - 1] |= attacks;

	// get incremental material scores first
	int score = pos->material[WHITE] - pos->material[BLACK];

	// get big pieces first, to get all attacks in eval info before king safety eval
	score += evalKnights(pos, eval_info, params, WHITE) - evalKnights(pos, eval_info, params, BLACK) +
			 evalBishops(pos, eval_info, params, WHITE) - evalBishops(pos, eval_info, params, BLACK) +
			 evalRooks(pos, eval_info, params, WHITE)   - evalRooks(pos, eval_info, params, BLACK)   +
			 evalQueens(pos, eval_info, params, WHITE)  - evalQueens(pos, eval_info, params, BLACK);

	// finally, after all attacks are calculated, get kings and pawns
	score += evalKings(pos, eval_info, params, WHITE) - evalKings(pos, eval_info, params, BLACK) + 
			 evalPawns(pos, eval_info, params, WHITE) - evalPawns(pos, eval_info, params, BLACK);

	// then evaluate threats in the position
	score += evalThreats(pos, eval_info, params, WHITE) - evalThreats(pos, eval_info, params, BLACK);
	
	// scale the endgame score based on other endgame factors
	int scale = evalScale(pos, score);

	// interpolate score between midgame and endgame via game phase
    int phase = queenPhase * (pos->pceNum[wQ] + pos->pceNum[bQ]) + 
          		rookPhase  * (pos->pceNum[wR] + pos->pceNum[bR]) + 
          		minorPhase * (pos->pceNum[wB] + pos->pceNum[bB]) + 
		  		minorPhase * (pos->pceNum[wN] + pos->pceNum[bN]);

	// get midgame and endgame score from packed int and return relative score by side
	score = (SCORE_MG(score) * phase + SCORE_EG(score) * (totalPhase - phase) * scale / SCALE_NORMAL) / totalPhase;
	return tempo + (pos->side == WHITE ? score : -score);
}
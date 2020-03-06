// search.c

#include "stdio.h"
#include "defs.h"
#include "math.h"

// Delta Pruning Value
static const int Delta = 1000;

// Null Move Pruning Values
static const int R = 2;
static const int minDepth = 3;

// Razoring Values
static const int RazorDepth = 2;
static const int RazorMargin[3] = {0, 200, 400};

// Futility Values
static const int FutilityDepth = 6;
static const int FutilityMargin[7] = {0, 200, 325, 450, 575, 700, 825};

// Reverse Futility Values
static const int RevFutilityDepth = 5;
static const int RevFutilityMargin[6] = {0, 200, 400, 600, 800, 1000};

// LMR Values
static const int LateMoveDepth = 3;
static const int FullSearchMoves = 2;
int LMRTable[64][64];

void InitSearch() {
	// creating the LMR table entries (idea from Ethereal)
	for (int moveDepth = 1; moveDepth < 64; moveDepth++)
  	for (int played = 1; played < 64; played++)
      LMRTable[moveDepth][played] = 1 + (log(moveDepth) * log(played) / 1.75);
}

static void CheckUp(S_SEARCHINFO *info) {
	// .. check if time up, or interrupt from GUI
	if(info->timeset == TRUE && GetTimeMs() > info->stoptime) {
		info->stopped = TRUE;
	}

	ReadInput(info);
}


static void SEEOrdering(S_MOVELIST *list, S_BOARD *pos) {

	// Calculate SEE for Captures (order losing captures after killer moves)

	for (int index = 0; index < list->count; ++index) {

		if (list->moves[index].move & MFLAGCAP) {
			int SEEval = SEECapture(list->moves[index].move, pos);

			// If SEE is negative, order after killer moves
			if (SEEval >= 0) {
				list->moves[index].score = 1000000 + SEEval;
			} else {
				list->moves[index].score = 800000 + SEEval;
			}

		}
	}
}


static void PickNextMove(int moveNum, S_MOVELIST *list, S_BOARD *pos) {

	S_MOVE temp;
	int index = 0;
	int bestScore = 0;
	int bestNum = moveNum;

	for (index = moveNum; index < list->count; ++index) {

		if (list->moves[index].score > bestScore) {
			bestScore = list->moves[index].score;
			bestNum = index;
		}
	}

	ASSERT(moveNum>=0 && moveNum<list->count);
	ASSERT(bestNum>=0 && bestNum<list->count);
	ASSERT(bestNum>=moveNum);

	temp = list->moves[moveNum];
	list->moves[moveNum] = list->moves[bestNum];
	list->moves[bestNum] = temp;
}

static int IsRepetition(const S_BOARD *pos) {

	int index = 0;

	for(index = pos->hisPly - pos->fiftyMove; index < pos->hisPly-1; ++index) {
		ASSERT(index >= 0 && index < MAXGAMEMOVES);
		if(pos->posKey == pos->history[index].posKey) {
			return TRUE;
		}
	}
	return FALSE;
}

static void ClearForSearch(S_BOARD *pos, S_SEARCHINFO *info) {

	int index = 0;
	int index2 = 0;

	for(index = 0; index < 13; ++index) {
		for(index2 = 0; index2 < BRD_SQ_NUM; ++index2) {
			pos->searchHistory[index][index2] = 0;
		}
	}

	for(index = 0; index < 2; ++index) {
		for(index2 = 0; index2 < MAXDEPTH; ++index2) {
			pos->searchKillers[index][index2] = 0;
		}
	}

	pos->HashTable->overWrite=0;
	pos->HashTable->hit=0;
	pos->HashTable->cut=0;
	pos->ply = 0;

	info->stopped = 0;
	info->nodes = 0;
	info->fh = 0;
	info->fhf = 0;
}

static int Quiescence(int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info) {

	ASSERT(CheckBoard(pos));
	ASSERT(beta>alpha);
	if(( info->nodes & 2047 ) == 0) {
		CheckUp(info);
	}

	info->nodes++;

	if(IsRepetition(pos) || pos->fiftyMove >= 100) {
		return 0;
	}

	if(pos->ply > MAXDEPTH - 1) {
		return EvalPosition(pos);
	}

	// Mate Distance Pruning
	alpha = MAX(alpha, -INFINITE + pos->ply);
	beta = MIN(beta, INFINITE - pos->ply);
	if (alpha >= beta) {
		return alpha;
	}

	int Score = EvalPosition(pos);

	ASSERT(Score>-INFINITE && Score<INFINITE);

	if(Score >= beta) {
		return beta;
	}
/*
	// delta pruning (test if any capture can improve position)
	if (Score + Delta < alpha) {
		// if no move can improve alpha, return
		return alpha;
	}
*/
	if(Score > alpha) {
		alpha = Score;
	}

	S_MOVELIST list[1];
    GenerateAllCaps(pos,list);

    int MoveNum = 0;
	int Legal = 0;
	Score = -INFINITE;

	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		PickNextMove(MoveNum, list, pos);

        if ( !MakeMove(pos,list->moves[MoveNum].move))  {
            continue;
        }

		Legal++;
		Score = -Quiescence( -beta, -alpha, pos, info);
        TakeMove(pos);

		if(info->stopped == TRUE) {
			return 0;
		}

		if(Score > alpha) {
			if(Score >= beta) {
				if(Legal==1) {
					info->fhf++;
				}
				info->fh++;
				return beta;
			}
			alpha = Score;
		}
    }

	ASSERT(alpha >= OldAlpha);

	return alpha;
}

static int AlphaBeta(int alpha, int beta, int depth, S_BOARD *pos, S_SEARCHINFO *info, int DoNull, int DoLMR) {

	ASSERT(CheckBoard(pos));
	ASSERT(beta>alpha);
	ASSERT(depth>=0);

	int InCheck = SqAttacked(pos->KingSq[pos->side],pos->side^1,pos);

	// Check Extension (Extend all checks before dropping into Quiescence)
	if(InCheck) {
		depth++;
	}

	if(depth <= 0) {
		return Quiescence(alpha, beta, pos, info);
		// return EvalPosition(pos);
	}

	if(( info->nodes & 2047 ) == 0) {
		CheckUp(info);
	}

	info->nodes++;

	if((IsRepetition(pos) || pos->fiftyMove >= 100) && pos->ply) {
		return 0;
	}

	if(pos->ply > MAXDEPTH - 1) {
		return EvalPosition(pos);
	}

	// Mate Distance Pruning (finds mates more quickly)
	alpha = MAX(alpha, -INFINITE + pos->ply);
	beta = MIN(beta, INFINITE - pos->ply);
	if (alpha >= beta) {
		return alpha;
	}

	int Score = -INFINITE;
	int PvMove = NOMOVE;

	if( ProbeHashEntry(pos, &PvMove, &Score, alpha, beta, depth) == TRUE ) {
		pos->HashTable->cut++;
		return Score;
	}

	int positionEval = EvalPosition(pos);

	// Razoring (prunes near alpha)
	if (depth <= RazorDepth && !PvMove && !InCheck && positionEval + RazorMargin[depth] <= alpha) {
		// drop into qSearch if move most likely won't beat alpha
		Score = Quiescence(alpha - RazorMargin[depth], beta - RazorMargin[depth], pos, info);
		if (Score + RazorMargin[depth] <= alpha) {
			return Score;
		}
	}

	// Reverse Futility Pruning (prunes near beta)
	if (depth <= RevFutilityDepth && !PvMove && !InCheck && abs(beta) < ISMATE && positionEval - RevFutilityMargin[depth] >= beta) {
		return positionEval - RevFutilityMargin[depth];
	}

	// Null Move Pruning
	if(depth >= minDepth && DoNull && !InCheck && pos->ply && (pos->bigPce[pos->side] > 0) && positionEval >= beta) {
		MakeNullMove(pos);
		Score = -AlphaBeta( -beta, -beta + 1, depth - 1 - R, pos, info, FALSE, FALSE);
		TakeNullMove(pos);
		if(info->stopped == TRUE) {
			return 0;
		}

		if (Score >= beta && abs(Score) < ISMATE) {
			info->nullCut++;
			return beta;
		}
	}

	S_MOVELIST list[1];
  GenerateAllMoves(pos,list);

  int MoveNum = 0;
	int Legal = 0;
	int OldAlpha = alpha;
	int BestMove = NOMOVE;

	int BestScore = -INFINITE;

	Score = -INFINITE;

	if( PvMove != NOMOVE) {
		for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
			if( list->moves[MoveNum].move == PvMove) {
				list->moves[MoveNum].score = 2000000;
				//printf("Pv move found \n");
				break;
			}
		}
	}

	int FoundPv = FALSE;

	// Futility Pruning flag (if node is futile (unlikely to raise alpha), this flag is set)
	int FutileNode = (depth <= FutilityDepth && positionEval + FutilityMargin[depth] <= alpha && abs(Score) < ISMATE) ? 1 : 0;

	// before entering the move loop, calculate capture scores using SEE (can use these move ordering scores for reductions in LMR)
	SEEOrdering(list, pos);

	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		PickNextMove(MoveNum, list, pos);

		// Futility Pruning (if node is considered futile, and at least 1 legal move has been searched, don't search any more quiet moves in the position)
		if (Legal && FutileNode && !(list->moves[MoveNum].move & MFLAGCAP) && !(list->moves[MoveNum].move & MFLAGPROM) && !SqAttacked(pos->KingSq[pos->side],pos->side^1,pos)) {
			continue;
		}

		// if move is legal, play it
    if ( !MakeMove(pos,list->moves[MoveNum].move))  {
        continue;
    }

		Legal++;

		// PVS (speeds up search with good move ordering)
		if (FoundPv == TRUE) {

			// Late Move Reductions at Root (reduces moves if past full move search limit (not reducing captures, checks, or promotions))
			if (depth >= LateMoveDepth && !(list->moves[MoveNum].move & MFLAGCAP) && !(list->moves[MoveNum].move & MFLAGPROM) && !SqAttacked(pos->KingSq[pos->side],pos->side^1,pos) && DoLMR && Legal > FullSearchMoves) {

				// get initial reduction depth
				int reduce = LMRTable[MIN(depth, 63)][MIN(Legal, 63)];

				// reduce less for killer moves
				if ((list->moves[MoveNum].score == 800000 || list->moves[MoveNum].score == 900000)) reduce--;

				// do not fall directly into quiescence search
				reduce = MIN(depth - 1, MAX(reduce, 1));

				// print reduction depth at move number
				// printf("reduction: %d depth: %d moveNum: %d\n", (reduce - 1), depth, Legal);

				// search with the reduced depth
				Score = -AlphaBeta( -alpha - 1, -alpha, depth - reduce, pos, info, TRUE, FALSE);

			} else {
				// If LMR conditions not met (not at root, or tactical move), do a null window search (because we are using PVS)
				Score = -AlphaBeta( -alpha - 1, -alpha, depth - 1, pos, info, TRUE, TRUE);

			}
			if (Score > alpha && Score < beta) {
				// If the LMR or the null window fails, do a full search
				Score = -AlphaBeta( -beta, -alpha, depth - 1, pos, info, TRUE, FALSE);

			}
		} else {
			// If no PV found, do a full search
			Score = -AlphaBeta( -beta, -alpha, depth - 1, pos, info, TRUE, FALSE);

		}

		TakeMove(pos);

		if(info->stopped == TRUE) {
			return 0;
		}
		if(Score > BestScore) {
			BestScore = Score;
			BestMove = list->moves[MoveNum].move;
			if(Score > alpha) {
				if(Score >= beta) {
					if(Legal==1) {
						info->fhf++;
					}
					info->fh++;

					if(!(list->moves[MoveNum].move & MFLAGCAP)) {
						pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
						pos->searchKillers[0][pos->ply] = list->moves[MoveNum].move;
					}

					StoreHashEntry(pos, BestMove, beta, HFBETA, depth);

					return beta;
				}
				FoundPv = TRUE;
				alpha = Score;

				if(!(list->moves[MoveNum].move & MFLAGCAP)) {
					pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] += depth;
				}
			}
		}
    }

	if(Legal == 0) {
		if(InCheck) {
			return -INFINITE + pos->ply;
		} else {
			return 0;
		}
	}

	ASSERT(alpha>=OldAlpha);

	if(alpha != OldAlpha) {
		StoreHashEntry(pos, BestMove, BestScore, HFEXACT, depth);
	} else {
		StoreHashEntry(pos, BestMove, alpha, HFALPHA, depth);
	}

	return alpha;
}

void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info) {

	int bestMove = NOMOVE;
	int bestScore = -INFINITE;
	int currentDepth = 0;
	int pvMoves = 0;
	int pvNum = 0;

	ClearForSearch(pos,info);

	//printf("Search depth:%d\n",info->depth);

	// iterative deepening
	for( currentDepth = 1; currentDepth <= info->depth; ++currentDepth ) {
							// alpha	 beta
		bestScore = AlphaBeta(-INFINITE, INFINITE, currentDepth, pos, info, TRUE, TRUE);

		if(info->stopped == TRUE) {
			break;
		}

		pvMoves = GetPvLine(currentDepth, pos);
		bestMove = pos->PvArray[0];
		if(info->GAME_MODE == UCIMODE) {
			if (abs(bestScore) > ISMATE) {
				bestScore = (bestScore > 0 ? INFINITE - bestScore + 1 : -INFINITE - bestScore) / 2;
				printf("info score mate %d depth %d nodes %ld time %d ",
					bestScore,currentDepth,info->nodes,GetTimeMs()-info->starttime);
				} else {
			printf("info score cp %d depth %d nodes %ld time %d ",
				bestScore,currentDepth,info->nodes,GetTimeMs()-info->starttime);
			}
		} else if(info->GAME_MODE == XBOARDMODE && info->POST_THINKING == TRUE) {
			printf("%d %d %d %ld ",
				currentDepth,bestScore,(GetTimeMs()-info->starttime)/10,info->nodes);
		} else if(info->POST_THINKING == TRUE) {
			printf("score:%d depth:%d nodes:%ld time:%d(ms) ",
				bestScore,currentDepth,info->nodes,GetTimeMs()-info->starttime);
		}
		if(info->GAME_MODE == UCIMODE || info->POST_THINKING == TRUE) {
			pvMoves = GetPvLine(currentDepth, pos);
			if(!info->GAME_MODE == XBOARDMODE) {
				printf("pv");
			}
			for(pvNum = 0; pvNum < pvMoves; ++pvNum) {
				printf(" %s",PrMove(pos->PvArray[pvNum]));
			}
			printf("\n");
		}
		/*
		Move statistics (like ordering, transpostion table stats, and null move stats)
		*/
		printf("Hits:%d Overwrite:%d NewWrite:%d Cut:%d\nOrdering %.2f NullCut:%d\n",pos->HashTable->hit,pos->HashTable->overWrite,pos->HashTable->newWrite,pos->HashTable->cut,
		(info->fhf/info->fh)*100,info->nullCut);
	}

	if(info->GAME_MODE == UCIMODE) {
		printf("bestmove %s\n",PrMove(bestMove));
	} else if(info->GAME_MODE == XBOARDMODE) {
		printf("move %s\n",PrMove(bestMove));
		MakeMove(pos, bestMove);
	} else {
		printf("\n\n***!! CeeChess makes move %s !!***\n\n",PrMove(bestMove));
		MakeMove(pos, bestMove);
		PrintBoard(pos);
	}

}

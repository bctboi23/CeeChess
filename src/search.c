// search.c

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "search.h"
#include "move.h"
#include "attack.h"
#include "movegen.h"
#include "evaluate.h"
#include "io.h"
#include "debug.h"
#include "misc.h"
#include "eval-tuned.h"

// Null Move Pruning Values
static const int R = 2;
static const int minDepth = 3;

// Razoring Values 
static const int RazorDepth = 2;
static const int RazorMargin[3] = {0, 200, 400};

// Futility Values
static const int FutilityDepth = 10;
static const int FutilityMargin = 150;

// Reverse Futility Values
static const int RevFutilityDepth = 10;
static const int RevFutilityMargin = 200;

// LMR Values
static const int LateMoveDepth = 3;
static const int FullSearchMoves = 1;
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

static void PickNextMove(int moveNum, S_MOVELIST *list) {

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

static void ClearForSearch(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table) {

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

	for (index = 0; index < MAXDEPTH; ++index) {
		pos->evalStack[index] = 0;
	}

	table->overWrite=0;
	table->hit=0;
	table->cut=0;
	pos->ply = 0;
	table->currentage++;

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
		return EvalPosition(pos, curr_params);
	}

	// Mate Distance Pruning
	alpha = MAX(alpha, -INFINITE + pos->ply);
	beta = MIN(beta, INFINITE - pos->ply);
	if (alpha >= beta) {
		return alpha;
	}

	int Score = EvalPosition(pos, curr_params);

	ASSERT(Score>-INFINITE && Score<INFINITE);

	if(Score >= beta) {
		return beta;
	}

	if (Score > alpha) {
		alpha = Score;
	}

	S_MOVELIST list[1];
    GenerateAllCaps(pos,list);

    int MoveNum = 0;
	int Legal = 0;
	Score = -INFINITE;

	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		// return immediately if stopped
		if(info->stopped == TRUE) {
			return beta;
		}

		PickNextMove(MoveNum, list);

        if ( !MakeMove(pos,list->moves[MoveNum].move))  {
            continue;
        }

		Legal++;
		Score = -Quiescence( -beta, -alpha, pos, info);
        TakeMove(pos);

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

	// ASSERT(alpha >= OldAlpha);

	return alpha;
}

static int AlphaBeta(int alpha, int beta, int depth, S_BOARD *pos, S_SEARCHINFO *info, int DoNull, int DoLMR, S_HASHTABLE *table) {

	ASSERT(CheckBoard(pos));
	ASSERT(beta>alpha);
	ASSERT(depth>=0);

	int InCheck = SqAttacked(pos->KingSq[pos->side],pos->side^1,pos);
	int isPv = (alpha != beta - 1);
	int rootNode = (pos->ply == 0);

	// Check Extension (Extend all checks before dropping into Quiescence)
	if (!rootNode && InCheck) {
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

	// check for draws
	if((IsRepetition(pos) || pos->fiftyMove >= 100) && pos->ply) {
		return 0;
	}

	if(pos->ply > MAXDEPTH - 1) {
		return EvalPosition(pos, curr_params);
	}

	// Mate Distance Pruning (finds mates more quickly)
	alpha = MAX(alpha, -INFINITE + pos->ply);
	beta = MIN(beta, INFINITE - pos->ply); 
	if (alpha >= beta) {
		return alpha;
	}

	int Score = -INFINITE;
	int PvMove = NOMOVE;
	int ttDepth = 0;

	int probeHash = ProbeHashEntry(pos, table, &PvMove, &Score, &ttDepth, alpha, beta, depth);
	if(!isPv && probeHash == TRUE && pos->fiftyMove < 90) {
		table->cut++;
		return Score;
	}

	int positionEval = EvalPosition(pos, curr_params);
	pos->evalStack[pos->ply] = InCheck ? INFINITE : positionEval; // don't save evaluation when in check

	int improving = (pos->ply >= 2 && 
					!InCheck && 
					pos->evalStack[pos->ply - 2] < positionEval);

	// Razoring (prunes near alpha)
	if (depth <= RazorDepth && !isPv && !InCheck && positionEval + RazorMargin[depth] <= alpha) {
		// drop into qSearch if move most likely won't beat alpha
		Score = Quiescence(alpha - RazorMargin[depth], beta + RazorMargin[depth], pos, info);
		if (Score + RazorMargin[depth] <= alpha) {
			return Score + RazorMargin[depth];
		}
	}

	// Reverse Futility Pruning (prunes near beta)
	if (depth <= RevFutilityDepth && !isPv && !InCheck && abs(beta) < ISMATE && positionEval - (RevFutilityMargin * depth) >= beta) {
		return positionEval - (RevFutilityMargin * (depth - improving));
	}

	// Null Move Pruning
	if(!isPv && depth >= minDepth && DoNull && !InCheck && pos->ply && (pos->bigPce[pos->side] > 0) && positionEval >= beta) {
		MakeNullMove(pos);
		Score = -AlphaBeta( -beta, -beta + 1, depth - 1 - R, pos, info, FALSE, FALSE, table);
		TakeNullMove(pos);
		if(info->stopped == TRUE) {
			return beta;
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

	// If the hash table found a move but couldn't directly cut, we search it first
	if( PvMove != NOMOVE) {
		for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
			if( list->moves[MoveNum].move == PvMove) {
				list->moves[MoveNum].score = 2000000;
				//printf("Pv move found \n");
				break;
			}
		}
	}

	// if the hash table has no move, or the move found has a depth much lower than current depth, we reduce the search depth (IIR)
	if (depth >= 4 && (!PvMove || ttDepth + 4 <= depth))
        --depth;

	int FoundPv = FALSE;

	// Futility Pruning flag (if node is futile (unlikely to raise alpha), this flag is set)
	int FutileNode = (!isPv && 
					depth <= FutilityDepth && 
					positionEval + (FutilityMargin * (depth - improving)) <= alpha && 
					abs(Score) < ISMATE && 
					(pos->bigPce[pos->side] > 0));

	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		// return if stopped
		if(info->stopped == TRUE) {
			return beta;
		}

		PickNextMove(MoveNum, list);

		// Futility Pruning (if node is considered futile, and at least 1 legal move has been searched, don't search any more quiet moves in the position)
		int isMoveCheck = SqAttacked(pos->KingSq[pos->side^1],pos->side,pos);
		int nonCaptureProm = !(list->moves[MoveNum].move & MFLAGCAP) && !(list->moves[MoveNum].move & MFLAGPROM);
		int isQuiet = (nonCaptureProm && !isMoveCheck);
		if (Legal && FutileNode && isQuiet && abs(Score) < ISMATE) { // ) {
			continue;
		}

		// if move is legal, play it
		if ( !MakeMove(pos,list->moves[MoveNum].move))  {
			continue;
		}

		Legal++;

		// PVS (speeds up search with good move ordering)
		if (FoundPv == TRUE) {

			// Late Move Reductions (reduces quiet moves late in the search)
			if (depth >= LateMoveDepth && Legal > FullSearchMoves && !InCheck && isQuiet && DoLMR) {

				// get initial reduction depth
				int reduce = LMRTable[MIN(depth, 63)][MIN(Legal, 63)];

				// reduce less for killer moves
				if ((list->moves[MoveNum].move == pos->searchKillers[0][pos->ply]) || 
					(list->moves[MoveNum].move == pos->searchKillers[1][pos->ply])
				) reduce--;

				// reduce more if we are not in the PV and/or not improving
				reduce += !isPv + !improving;

				// do not fall directly into quiescence search
				reduce = CLAMP(reduce, 1, depth - 1);

				// print reduction depth at move number
				// printf("reduction: %d depth: %d moveNum: %d\n", (reduce - 1), depth, Legal);

				// search with the reduced depth
				Score = -AlphaBeta( -alpha - 1, -alpha, depth - reduce, pos, info, TRUE, FALSE, table);

				// If the LMR fails, do a full depth null-window search
				if (Score > alpha && Score < beta) {
					Score = -AlphaBeta( -alpha - 1, -alpha, depth - 1, pos, info, TRUE, FALSE, table);
				}
			} else {
				// If LMR conditions not met, do a null window search (because we are using PVS)
				Score = -AlphaBeta( -alpha - 1, -alpha, depth - 1, pos, info, TRUE, DoLMR, table);
			}
			// If the null window fails, do a full window search
			if (Score > alpha && Score < beta) {
				Score = -AlphaBeta( -beta, -alpha, depth - 1, pos, info, TRUE, FALSE, table);
			}

		} else {
			// If no PV found, do a full search
			Score = -AlphaBeta( -beta, -alpha, depth - 1, pos, info, TRUE, FALSE, table);
		}

		TakeMove(pos);

		if(Score > BestScore) {
			BestScore = Score;
			BestMove = list->moves[MoveNum].move;
		}
		if(Score > alpha) {
			if(Score >= beta) {
				if(Legal==1) {
					info->fhf++;
				}
				info->fh++;
				if (nonCaptureProm) {
					if ((pos->searchKillers[0][pos->ply] != list->moves[MoveNum].move)) {
						pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
						pos->searchKillers[0][pos->ply] = list->moves[MoveNum].move;
					}
				}
				StoreHashEntry(pos, table, BestMove, beta, HFBETA, depth);
				return beta;
			}
			FoundPv = TRUE;
			alpha = Score;
			if (nonCaptureProm) {
				// possible new history formula
				/*
				int bonus = MIN(depth * depth, 128);
				pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] += bonus - pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] * bonus / 4096;
				*/
				pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] += depth;
			}
		}
    }

	if(Legal == 0) {
		return InCheck * (-INFINITE + pos->ply);
	}

	ASSERT(alpha>=OldAlpha);

	// store in hash table
	if(alpha != OldAlpha) {
		StoreHashEntry(pos, table, BestMove, BestScore, HFEXACT, depth);
	} else {
		StoreHashEntry(pos, table, BestMove, alpha, HFALPHA, depth);
	}

	return alpha;
}

void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table) {

	int bestMove = NOMOVE;
	int bestScore = -INFINITE;
	int currentDepth = 0;
	int pvMoves = 0;
	int pvNum = 0;

	ClearForSearch(pos, info, table);

	//printf("Search depth:%d\n",info->depth);

	// iterative deepening
	for( currentDepth = 1; currentDepth <= info->depth; ++currentDepth ) {
							// alpha	 beta
		bestScore = AlphaBeta(-INFINITE, INFINITE, currentDepth, pos, info, TRUE, TRUE, table);
		pvMoves = GetPvLine(currentDepth, pos, table);
		bestMove = pos->PvArray[0];

		if(info->stopped == TRUE) {
			break;
		}

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
			if((!info->GAME_MODE) == XBOARDMODE) {
				printf("pv");
			}
			for(pvNum = 0; pvNum < pvMoves; ++pvNum) {
				printf(" %s",PrMove(pos->PvArray[pvNum]));
			}
			printf("\n");
		}

		// end an iteration early if we are unlikely to complete a full depth on the next search
		if (info->timeset == TRUE && GetTimeMs() > info->earlyend) {
			info->stopped = TRUE;
			break;
		}

		//printf("Hits:%d Overwrite:%d NewWrite:%d Cut:%d\nOrdering %.2f NullCut:%d",table->hit,table->overWrite,table->newWrite,table->cut,
		//(info->fhf/info->fh)*100,info->nullCut);
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

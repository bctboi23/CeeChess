// search.c

#include "stdio.h"
#include "defs.h"

#define INFINITE 30000
#define MATE 29000

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
	
	ClearPvTable(pos->PvTable);	
	pos->ply = 0;
	
	info->stopped = 0;
	info->nodes = 0;
	info->fh = 0;
	info->fhf = 0;
}

static int Quiescence(int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info) {

	ASSERT(CheckBoard(pos));
	
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
	
	int Score = EvalPosition(pos);
	
	if(Score >= beta) {
		return beta;
	}
	
	if(Score > alpha) {
		alpha = Score;
	}
	
	S_MOVELIST list[1];
    GenerateAllCaps(pos,list);
      
    int MoveNum = 0;
	int Legal = 0;
	int OldAlpha = alpha;
	int BestMove = NOMOVE;
	Score = -INFINITE;
	int PvMove = ProbePvTable(pos);
	
	if( PvMove != NOMOVE ) {
		for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
			if( list->moves[MoveNum].move == PvMove) {
				list->moves[MoveNum].score = 2000000;
				break;
			}
		}
	}
	
	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {	
			
		PickNextMove(MoveNum, list);	
		
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
			BestMove = list->moves[MoveNum].move;
		}		
    }
	
	if(alpha != OldAlpha) {
		StorePvMove(pos, BestMove);
	}
	
	return alpha;
}

static int AlphaBeta(int alpha, int beta, int depth, S_BOARD *pos, S_SEARCHINFO *info, int DoNull) {

	ASSERT(CheckBoard(pos)); 
	
	if(depth == 0) {
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
	
	S_MOVELIST list[1];
    GenerateAllMoves(pos,list);
      
    int MoveNum = 0;
	int Legal = 0;
	int OldAlpha = alpha;
	int BestMove = NOMOVE;
	int Score = -INFINITE;
	int PvMove = ProbePvTable(pos);
	
	if( PvMove != NOMOVE ) {
		for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
			if( list->moves[MoveNum].move == PvMove) {
				list->moves[MoveNum].score = 2000000;
				break;
			}
		}
	}
	
	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {	
			
		PickNextMove(MoveNum, list);	
		
        if ( !MakeMove(pos,list->moves[MoveNum].move))  {
            continue;
        }
        
		Legal++;
		Score = -AlphaBeta( -beta, -alpha, depth-1, pos, info, TRUE);		
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
				
				if(!(list->moves[MoveNum].move & MFLAGCAP)) {
					pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
					pos->searchKillers[0][pos->ply] = list->moves[MoveNum].move;
				}
				
				return beta;
			}
			alpha = Score;
			BestMove = list->moves[MoveNum].move;
			if(!(list->moves[MoveNum].move & MFLAGCAP)) {
				pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] += depth;
			}
		}		
    }
	
	if(Legal == 0) {
		if(SqAttacked(pos->KingSq[pos->side],pos->side^1,pos)) {
			return -MATE + pos->ply;
		} else {
			return 0;
		}
	}
	
	if(alpha != OldAlpha) {
		StorePvMove(pos, BestMove);
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
	
	// iterative deepening
	for( currentDepth = 1; currentDepth <= info->depth; ++currentDepth ) {
							// alpha	 beta
		bestScore = AlphaBeta(-INFINITE, INFINITE, currentDepth, pos, info, TRUE);
		
		if(info->stopped == TRUE) {
			break;
		}
		
		pvMoves = GetPvLine(currentDepth, pos);
		bestMove = pos->PvArray[0];
		
		printf("info score cp %d depth %d nodes %ld time %d ",
			bestScore,currentDepth,info->nodes,GetTimeMs()-info->starttime);
			
		pvMoves = GetPvLine(currentDepth, pos);	
		printf("pv");		
		for(pvNum = 0; pvNum < pvMoves; ++pvNum) {
			printf(" %s",PrMove(pos->PvArray[pvNum]));
		}
		printf("\n");
		// printf("Ordering:%.2f\n",(info->fhf/info->fh));
	}
	//info score cp 13  depth 1 nodes 13 time 15 pv f1b5
	printf("bestmove %s\n",PrMove(bestMove));
	
}





















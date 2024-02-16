// evaluate.c

#include "stdio.h"
#include "defs.h"
#include "eval-tuned.h"

int DistTable[64][64];

void InitEval() {
	for (int i = 0; i < 64; ++i) {
		for (int j = 0; j < 64; ++j) {
			DistTable[i][j] = 14 - ( abs ( COL(i) - COL(j) ) + abs ( ROW(i) - ROW(j) ) );
      	}
   	}
}

static int MaterialDraw(const S_BOARD *pos) {

	ASSERT(CheckBoard(pos));

    if (!pos->pceNum[wR] && !pos->pceNum[bR] && !pos->pceNum[wQ] && !pos->pceNum[bQ]) {
	  if (!pos->pceNum[bB] && !pos->pceNum[wB]) {
	      if (pos->pceNum[wN] < 3 && pos->pceNum[bN] < 3) {  return TRUE; }
	  } else if (!pos->pceNum[wN] && !pos->pceNum[bN]) {
	     if (abs(pos->pceNum[wB] - pos->pceNum[bB]) < 2) { return TRUE; }
	  } else if ((pos->pceNum[wN] < 3 && !pos->pceNum[wB]) || (pos->pceNum[wB] == 1 && !pos->pceNum[wN])) {
	    if ((pos->pceNum[bN] < 3 && !pos->pceNum[bB]) || (pos->pceNum[bB] == 1 && !pos->pceNum[bN]))  { return TRUE; }
	  }
	} else if (!pos->pceNum[wQ] && !pos->pceNum[bQ]) {
        if (pos->pceNum[wR] == 1 && pos->pceNum[bR] == 1) {
            if ((pos->pceNum[wN] + pos->pceNum[wB]) < 2 && (pos->pceNum[bN] + pos->pceNum[bB]) < 2)	{ return TRUE; }
        } else if (pos->pceNum[wR] == 1 && !pos->pceNum[bR]) {
            if ((pos->pceNum[wN] + pos->pceNum[wB] == 0) && (((pos->pceNum[bN] + pos->pceNum[bB]) == 1) || ((pos->pceNum[bN] + pos->pceNum[bB]) == 2))) { return TRUE; }
        } else if (pos->pceNum[bR] == 1 && !pos->pceNum[wR]) {
            if ((pos->pceNum[bN] + pos->pceNum[bB] == 0) && (((pos->pceNum[wN] + pos->pceNum[wB]) == 1) || ((pos->pceNum[wN] + pos->pceNum[wB]) == 2))) { return TRUE; }
        }
    }
  return FALSE;
}

inline int EvalPosition(S_BOARD *pos) {

	ASSERT(CheckBoard(pos));

	// test for drawn position before doing anything
	if(!pos->pceNum[wP] && !pos->pceNum[bP] && MaterialDraw(pos) == TRUE) {
		return 0;
	}

	int pce;
	int pceNum;
	int sq;
	int phase = totalPhase;
	//int mobility = GetMobility(pos, WHITE) - GetMobility(pos, BLACK);
	int scoreMG = (pos->material[WHITE] - pos->material[BLACK]); //+ (mobilityFactorMG * mobility);
	int scoreEG = (pos->material[WHITE + 2] - pos->material[BLACK + 2]); //+ (mobilityFactorEG * mobility);

	pce = wP;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);

		int passed = 0;
		int connected = 0;

		scoreMG += PawnMG[SQ64(sq)];
		scoreEG += PawnEG[SQ64(sq)];

		if( (IsolatedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
			//printf("wP Iso:%s\n",PrSq(sq));
			scoreMG += PawnIsolatedMG;
			scoreEG += PawnIsolatedEG;
		}

		if( (WhitePassedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
			//printf("wP Passed:%s\n",PrSq(sq));
			passed = 1;
			scoreMG += PawnPassedMG[RanksBrd[sq]];
			scoreEG += PawnPassedEG[RanksBrd[sq]];
		}

		if ((WhiteConnectedMask[SQ64(sq)] & pos->pawns[WHITE]) != 0) {
			connected = 1;
			scoreMG += PawnConnectedMG;
			scoreEG += PawnConnectedEG;
		}

		if (passed && connected) {
			scoreMG += PawnPassedConnectedMG[RanksBrd[sq]];
			scoreEG += PawnPassedConnectedEG[RanksBrd[sq]];
		}
	}

	pce = bP;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		int passed = 0;
		int connected = 0;

		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq))>=0 && MIRROR64(SQ64(sq))<=63);

		scoreMG -= PawnMG[MIRROR64(SQ64(sq))];
		scoreEG -= PawnEG[MIRROR64(SQ64(sq))];

		if( (IsolatedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
			//printf("bP Iso:%s\n",PrSq(sq));
			scoreMG -= PawnIsolatedMG;
			scoreEG -= PawnIsolatedEG;
		}

		if( (BlackPassedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
			//printf("bP Passed:%s\n",PrSq(sq));
			passed = 1;
			scoreMG -= PawnPassedMG[7 - RanksBrd[sq]];
			scoreEG -= PawnPassedEG[7 - RanksBrd[sq]];
		}

		if ((BlackConnectedMask[SQ64(sq)] & pos->pawns[BLACK]) != 0) {
			connected = 1;
			scoreMG -= PawnConnectedMG;
			scoreEG -= PawnConnectedEG;
		}

		if (passed && connected) {
			scoreMG -= PawnPassedConnectedMG[7 - RanksBrd[sq]];
			scoreEG -= PawnPassedConnectedEG[7 - RanksBrd[sq]];
		}
	}

	pce = wN;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);

		scoreMG += KnightMG[SQ64(sq)];
		scoreEG += KnightEG[SQ64(sq)];
		phase -= minorPhase;
	}

	pce = bN;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq))>=0 && MIRROR64(SQ64(sq))<=63);

		scoreMG -= KnightMG[MIRROR64(SQ64(sq))];
		scoreEG -= KnightEG[MIRROR64(SQ64(sq))];
		phase -= minorPhase;
	}

	pce = wB;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);

		scoreMG += BishopMG[SQ64(sq)];
		scoreEG += BishopEG[SQ64(sq)];
		phase -= minorPhase;
	}

	pce = bB;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq))>=0 && MIRROR64(SQ64(sq))<=63);

		scoreMG -= BishopMG[MIRROR64(SQ64(sq))];
		scoreEG -= BishopEG[MIRROR64(SQ64(sq))];
		phase -= minorPhase;
	}

	pce = wR;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		ASSERT(FileRankValid(FilesBrd[sq]));

		scoreMG += RookMG[SQ64(sq)];
		scoreEG += RookEG[SQ64(sq)];

		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			scoreMG += RookOpenFileMG;
		} else if(!(pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]])) {
			scoreMG += RookSemiOpenFileMG;
		}
		phase -= rookPhase;
	}

	pce = bR;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq))>=0 && MIRROR64(SQ64(sq))<=63);
		ASSERT(FileRankValid(FilesBrd[sq]));

		scoreMG -= RookMG[MIRROR64(SQ64(sq))];
		scoreEG -= RookEG[MIRROR64(SQ64(sq))];

		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			scoreMG -= RookOpenFileMG;
		} else if(!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
			scoreMG -= RookSemiOpenFileMG;
		}
		phase -= rookPhase;
	}

	pce = wQ;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		ASSERT(FileRankValid(FilesBrd[sq]));

		scoreMG += QueenMG[SQ64(sq)];
		scoreEG += QueenEG[SQ64(sq)];

		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			scoreMG += QueenOpenFileMG;
		} else if(!(pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]])) {
			scoreMG += QueenSemiOpenFileMG;
		}
		phase -= queenPhase;
	}

	pce = bQ;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		ASSERT(FileRankValid(FilesBrd[sq]));

		scoreMG -= QueenMG[MIRROR64(SQ64(sq))];
		scoreEG -= QueenEG[MIRROR64(SQ64(sq))];

		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			scoreMG -= QueenOpenFileMG;
		} else if(!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
			scoreMG -= QueenSemiOpenFileMG;
		}
		phase -= queenPhase;
	}
	//8/p6k/6p1/5p2/P4K2/8/5pB1/8 b - - 2 62
	pce = wK;
	sq = pos->pList[pce][0];
	ASSERT(SqOnBoard(sq));
	ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);

	scoreMG += KingMG[SQ64(sq)];
	scoreEG += KingEG[SQ64(sq)];

	pce = bK;
	sq = pos->pList[pce][0];
	ASSERT(SqOnBoard(sq));
	ASSERT(MIRROR64(SQ64(sq))>=0 && MIRROR64(SQ64(sq))<=63);

	scoreMG -= KingMG[MIRROR64(SQ64(sq))];
	scoreEG -= KingEG[MIRROR64(SQ64(sq))];

	if(pos->pceNum[wB] >= 2) {
		scoreMG += BishopPairMG;
		scoreEG += BishopPairEG;
	}
	if(pos->pceNum[bB] >= 2) {
		scoreMG -= BishopPairMG;
		scoreEG -= BishopPairEG;
	}

	//scoreMG += (pos->side == WHITE) ? tempoMG : -tempoMG;
	//scoreEG += (pos->side == WHITE) ? tempoEG : -tempoEG;

	// calculating game phase and interpolating score values between phases
	phase = (phase * 256 + (totalPhase / 2)) / totalPhase;
	int score = ((scoreMG * (256 - phase)) + (scoreEG * phase)) / 256;

	return pos->side == WHITE ? score : -score;

}
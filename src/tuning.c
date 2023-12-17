// evaluate.c

#include "stdio.h"
#include "defs.h"
#include "tuning.h"
#include "string.h"
#include "math.h"
#include "time.h"

static int MaterialDrawTunable(const S_BOARD *pos) {

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

static int EvalPositionTunable(S_BOARD *pos, S_EVAL_PARAMS *params) {

	ASSERT(CheckBoard(pos));

	// test for drawn position before doing anything
	if(!pos->pceNum[wP] && !pos->pceNum[bP] && MaterialDrawTunable(pos) == TRUE) {
		return 0;
	}

	int pce;
	int pceNum;
	int sq;
	int phase = TtotalPhase;
	int wPhase = TtotalPhase / 2;
	int bPhase = TtotalPhase / 2;
	//int mobility = GetMobility(pos, WHITE) - GetMobility(pos, BLACK);
	int scoreMG = 0; //(pos->material[WHITE] - pos->material[BLACK]); //+ (mobilityFactorMG * mobility);
	int scoreEG = 0; //(pos->material[WHITE + 2] - pos->material[BLACK + 2]); //+ (mobilityFactorEG * mobility);

	int kingScoreW = 0;
	int kingScoreB = 0;

	// get king squares to calculate king tropism
	pce = wK;
	sq = pos->pList[pce][0];
	int wKsq64 = SQ64(sq);
	ASSERT(SqOnBoard(sq));
	ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);

	scoreMG += params->TKingMG[SQ64(sq)];
	scoreEG += params->TKingEG[SQ64(sq)];

	pce = bK;
	sq = pos->pList[pce][0];
	int bKsq64 = SQ64(sq);
	ASSERT(SqOnBoard(sq));
	ASSERT(MIRROR64(SQ64(sq))>=0 && MIRROR64(SQ64(sq))<=63);

	scoreMG -= params->TKingMG[MIRROR64(SQ64(sq))];
	scoreEG -= params->TKingEG[MIRROR64(SQ64(sq))];

	pce = wP;
	scoreMG += pos->pceNum[pce] * params->TPieceValMG[0];
	scoreEG += pos->pceNum[pce] * params->TPieceValEG[0];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		int passed = 0;
		int connected = 0;

		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);

		scoreMG += params->TPawnMG[SQ64(sq)];
		scoreEG += params->TPawnEG[SQ64(sq)];

		if( (IsolatedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
			//printf("wP Iso:%s\n",PrSq(sq));
			scoreMG += params->TPawnIsolatedMG;
			scoreEG += params->TPawnIsolatedEG;
		}

		if( (WhitePassedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
			//printf("wP Passed:%s\n",PrSq(sq));
			passed = 1;
			scoreMG += params->TPawnPassedMG[RanksBrd[sq]];
			scoreEG += params->TPawnPassedEG[RanksBrd[sq]];
		}

		if ((WhiteConnectedMask[SQ64(sq)] & pos->pawns[WHITE]) != 0) {
			connected = 1;
			scoreMG += params->TPawnConnectedMG;
			scoreEG += params->TPawnConnectedEG;
		}

		if (passed && connected) {
			scoreMG += params->TPawnPassedConnectedMG[RanksBrd[sq]];
			scoreEG += params->TPawnPassedConnectedEG[RanksBrd[sq]];
		}

	}

	pce = bP;
	scoreMG -= pos->pceNum[pce] * params->TPieceValMG[0];
	scoreEG -= pos->pceNum[pce] * params->TPieceValEG[0];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		int passed = 0;
		int connected = 0;

		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq))>=0 && MIRROR64(SQ64(sq))<=63);

		scoreMG -= params->TPawnMG[MIRROR64(SQ64(sq))];
		scoreEG -= params->TPawnEG[MIRROR64(SQ64(sq))];

		if( (IsolatedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
			//printf("bP Iso:%s\n",PrSq(sq));
			scoreMG -= params->TPawnIsolatedMG;
			scoreEG -= params->TPawnIsolatedEG;
		}

		if( (BlackPassedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
			//printf("bP Passed:%s\n",PrSq(sq));
			passed = 1;
			scoreMG -= params->TPawnPassedMG[7 - RanksBrd[sq]];
			scoreEG -= params->TPawnPassedEG[7 - RanksBrd[sq]];
		}

		if ((BlackConnectedMask[SQ64(sq)] & pos->pawns[BLACK]) != 0) {
			connected = 1;
			scoreMG -= params->TPawnConnectedMG;
			scoreEG -= params->TPawnConnectedEG;
		}

		if (passed && connected) {
			scoreMG -= params->TPawnPassedConnectedMG[7 - RanksBrd[sq]];
			scoreEG -= params->TPawnPassedConnectedEG[7 - RanksBrd[sq]];
		}
	}

	pce = wN;
	scoreMG += pos->pceNum[pce] * params->TPieceValMG[1];
	scoreEG += pos->pceNum[pce] * params->TPieceValEG[1];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);

		scoreMG += params->TKnightMG[SQ64(sq)];
		scoreEG += params->TKnightEG[SQ64(sq)];
		phase -= TminorPhase;
		wPhase -= TminorPhase;

		kingScoreW += params->TTropismValues[0] * DistTable[SQ64(sq)][bKsq64] + params->TTropismAdjs[0];
	}

	pce = bN;
	scoreMG -= pos->pceNum[pce] * params->TPieceValMG[1];
	scoreEG -= pos->pceNum[pce] * params->TPieceValEG[1];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq))>=0 && MIRROR64(SQ64(sq))<=63);

		scoreMG -= params->TKnightMG[MIRROR64(SQ64(sq))];
		scoreEG -= params->TKnightEG[MIRROR64(SQ64(sq))];
		phase -= TminorPhase;
		bPhase -= TminorPhase;

		kingScoreB -= params->TTropismValues[0] * DistTable[SQ64(sq)][wKsq64] + params->TTropismAdjs[0];
	}

	pce = wB;
	scoreMG += pos->pceNum[pce] * params->TPieceValMG[2];
	scoreEG += pos->pceNum[pce] * params->TPieceValEG[2];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);

		scoreMG += params->TBishopMG[SQ64(sq)];
		scoreEG += params->TBishopEG[SQ64(sq)];
		phase -= TminorPhase;
		wPhase -= TminorPhase;

		kingScoreW += params->TTropismValues[1] * (DistTable[SQ64(sq)][bKsq64] + Tbonus_dia_distance[abs(Tdiag_ne[SQ64(sq)] - Tdiag_ne[bKsq64])]) + params->TTropismAdjs[1];
	}

	pce = bB;
	scoreMG -= pos->pceNum[pce] * params->TPieceValMG[2];
	scoreEG -= pos->pceNum[pce] * params->TPieceValEG[2];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq))>=0 && MIRROR64(SQ64(sq))<=63);

		scoreMG -= params->TBishopMG[MIRROR64(SQ64(sq))];
		scoreEG -= params->TBishopEG[MIRROR64(SQ64(sq))];
		phase -= TminorPhase;
		bPhase -= TminorPhase;

		kingScoreB -= params->TTropismValues[1] * (DistTable[SQ64(sq)][wKsq64] + Tbonus_dia_distance[abs(Tdiag_ne[SQ64(sq)] - Tdiag_ne[wKsq64])]) + params->TTropismAdjs[1];
	}

	pce = wR;
	scoreMG += pos->pceNum[pce] * params->TPieceValMG[3];
	scoreEG += pos->pceNum[pce] * params->TPieceValEG[3];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		ASSERT(FileRankValid(FilesBrd[sq]));

		scoreMG += params->TRookMG[SQ64(sq)];
		scoreEG += params->TRookEG[SQ64(sq)];

		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			scoreMG += params->TRookOpenFileMG;
			scoreEG += params->TRookOpenFileEG;
		} else if(!(pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]])) {
			scoreMG += params->TRookSemiOpenFileMG;
			scoreEG += params->TRookSemiOpenFileEG;
		}
		phase -= TrookPhase;

		kingScoreW += params->TTropismValues[2] * DistTable[SQ64(sq)][bKsq64] + params->TTropismAdjs[2];
	}

	pce = bR;
	scoreMG -= pos->pceNum[pce] * params->TPieceValMG[3];
	scoreEG -= pos->pceNum[pce] * params->TPieceValEG[3];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq))>=0 && MIRROR64(SQ64(sq))<=63);
		ASSERT(FileRankValid(FilesBrd[sq]));

		scoreMG -= params->TRookMG[MIRROR64(SQ64(sq))];
		scoreEG -= params->TRookEG[MIRROR64(SQ64(sq))];

		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			scoreMG -= params->TRookOpenFileMG;
			scoreEG -= params->TRookOpenFileEG;
		} else if(!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
			scoreMG -= params->TRookSemiOpenFileEG;
			scoreEG -= params->TRookSemiOpenFileEG;
		}
		phase -= TrookPhase;

		kingScoreB -= params->TTropismValues[2] * DistTable[SQ64(sq)][wKsq64] + params->TTropismAdjs[2];
	}

	pce = wQ;
	scoreMG += pos->pceNum[pce] * params->TPieceValMG[4];
	scoreEG += pos->pceNum[pce] * params->TPieceValEG[4];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		ASSERT(FileRankValid(FilesBrd[sq]));

		scoreMG += params->TQueenMG[SQ64(sq)];
		scoreEG += params->TQueenEG[SQ64(sq)];

		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			scoreMG += params->TQueenOpenFileMG;
			scoreEG += params->TQueenOpenFileEG;
		} else if(!(pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]])) {
			scoreMG += params->TQueenSemiOpenFileMG;
			scoreEG += params->TQueenSemiOpenFileEG;
		}
		phase -= TqueenPhase;

		kingScoreW += params->TTropismValues[3] * (DistTable[SQ64(sq)][bKsq64] + Tbonus_dia_distance[abs(Tdiag_ne[SQ64(sq)] - Tdiag_ne[bKsq64])]) + params->TTropismAdjs[3];
	}

	pce = bQ;
	scoreMG -= pos->pceNum[pce] * params->TPieceValMG[4];
	scoreEG -= pos->pceNum[pce] * params->TPieceValEG[4];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		ASSERT(FileRankValid(FilesBrd[sq]));

		scoreMG -= params->TQueenMG[MIRROR64(SQ64(sq))];
		scoreEG -= params->TQueenEG[MIRROR64(SQ64(sq))];

		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			scoreMG -= params->TQueenOpenFileMG;
			scoreEG -= params->TQueenOpenFileEG;
		} else if(!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
			scoreMG -= params->TQueenSemiOpenFileMG;
			scoreEG -= params->TQueenSemiOpenFileEG;
		}
		phase -= TqueenPhase;

		kingScoreB -= params->TTropismValues[3] * (DistTable[SQ64(sq)][wKsq64] + Tbonus_dia_distance[abs(Tdiag_ne[SQ64(sq)] - Tdiag_ne[wKsq64])]) + params->TTropismAdjs[3];
	}
	//8/p6k/6p1/5p2/P4K2/8/5pB1/8 b - - 2 62
	if(pos->pceNum[wB] >= 2) {
		scoreMG += params->TBishopPairMG;
		scoreEG += params->TBishopPairEG;
	}
	if(pos->pceNum[bB] >= 2) {
		scoreMG -= params->TBishopPairMG;
		scoreEG -= params->TBishopPairEG;
	}

	scoreMG += (pos->side == WHITE) ? params->TtempoMG : -params->TtempoMG;
	scoreEG += (pos->side == WHITE) ? params->TtempoEG : -params->TtempoEG;

	// scale king safety by material non-linearly
	kingScoreW *= params->TTropismMatAdjs[MIN(wPhase, 11)] / 256;
	kingScoreB *= params->TTropismMatAdjs[MIN(bPhase, 11)] / 256;

	scoreMG += kingScoreW + kingScoreB;
	scoreEG += kingScoreW + kingScoreB;

	// calculating game phase and interpolating score values between phases
	phase = (phase * 256 + (TtotalPhase / 2)) / TtotalPhase;
	int score = ((scoreMG * (256 - phase)) + (scoreEG * phase)) / 256;

	// For tuning, instead of returning the score based on the side to move, return the score in terms of white
	return score;

}

static void paramsToList(S_EVAL_PARAMS *params, int* paramsList) {
	memcpy(paramsList, params, sizeof(S_EVAL_PARAMS));
}

static void listToParams(int* paramsList, S_EVAL_PARAMS* params) {
	memcpy(params, paramsList, sizeof(S_EVAL_PARAMS));
}

static void printPSQTToFile(const int *array, FILE* file) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            fprintf(file, "%d, ", array[i * 8 + j]);
        }
        fprintf(file, "\n");
    }
	fprintf(file, "};\n");
}

static void printParamsToFile(S_EVAL_PARAMS *params, FILE *file) {
	fprintf(file, "Current Parameters: \n");
	for (int i = 0; i < 28; i++)
		fprintf(file, "-");
	fprintf(file, "\nconst int PieceValMG[5] = {");
    for (int i = 0; i < 5; ++i) {
        fprintf(file, "%d, ", params->TPieceValMG[i]);
    }
    fprintf(file, "};\nconst int BishopPairMG = %d;\n", params->TBishopPairMG);

    fprintf(file, "const int PawnPassedMG[8] = {");
    for (int i = 0; i < 8; ++i) {
        fprintf(file, "%d, ", params->TPawnPassedMG[i]);
    }
    fprintf(file, "};\nconst int PawnPassedConnectedMG[8] = {");
    for (int i = 0; i < 8; ++i) {
        fprintf(file, "%d, ", params->TPawnPassedConnectedMG[i]);
    }
    fprintf(file, "};\nconst int PawnConnectedMG = %d;\n", params->TPawnConnectedMG);
    fprintf(file, "const int PawnIsolatedMG = %d;\n", params->TPawnIsolatedMG);
    fprintf(file, "const int RookOpenFileMG = %d;\n", params->TRookOpenFileMG);
    fprintf(file, "const int RookSemiOpenFileMG = %d;\n", params->TRookSemiOpenFileMG);
    fprintf(file, "const int QueenOpenFileMG = %d;\n", params->TQueenOpenFileMG);
    fprintf(file, "const int QueenSemiOpenFileMG = %d;\n", params->TQueenSemiOpenFileMG);
    fprintf(file, "const int tempoMG = %d;\n", params->TtempoMG);

    fprintf(file, "const int PieceValEG[5] = {");
    for (int i = 0; i < 5; ++i) {
        fprintf(file, "%d, ", params->TPieceValEG[i]);
    }
    fprintf(file, "};\nconst int BishopPairEG = %d;\n", params->TBishopPairEG);

    fprintf(file, "const int PawnPassedEG[8] = {");
    for (int i = 0; i < 8; ++i) {
        fprintf(file, "%d, ", params->TPawnPassedEG[i]);
    }
    fprintf(file, "};\nconst int PawnPassedConnectedEG[8] = {");
    for (int i = 0; i < 8; ++i) {
        fprintf(file, "%d, ", params->TPawnPassedConnectedEG[i]);
    }
    fprintf(file, "};\nconst int PawnConnectedEG = %d;\n", params->TPawnConnectedEG);
    fprintf(file, "const int PawnIsolatedEG = %d;\n", params->TPawnIsolatedEG);
    fprintf(file, "const int RookOpenFileEG = %d;\n", params->TRookOpenFileEG);
    fprintf(file, "const int RookSemiOpenFileEG = %d;\n", params->TRookSemiOpenFileEG);
    fprintf(file, "const int QueenOpenFileEG = %d;\n", params->TQueenOpenFileEG);
    fprintf(file, "const int QueenSemiOpenFileEG = %d;\n", params->TQueenSemiOpenFileEG);
    fprintf(file, "const int tempoEG = %d;\n", params->TtempoEG);

	fprintf(file, "const int TTropismValues[4] = {");
    for (int i = 0; i < 4; ++i) {
        fprintf(file, "%d, ", params->TTropismValues[i]);
    }
    fprintf(file, "};\nconst int TTropismAdjs[4] = {");
    for (int i = 0; i < 4; ++i) {
        fprintf(file, "%d, ", params->TTropismAdjs[i]);
    }
	fprintf(file, "};\nconst int TTropismMatAdjs[13] = {");
    for (int i = 0; i < 13; ++i) {
        fprintf(file, "%d, ", params->TTropismMatAdjs[i]);
    }

    fprintf(file, "};\nconst int PawnMG[64] = {\n");
    printPSQTToFile(params->TPawnMG, file);
    fprintf(file, "const int PawnEG[64] = {\n");
    printPSQTToFile(params->TPawnEG, file);
    fprintf(file, "const int KnightMG[64] = {\n");
    printPSQTToFile(params->TKnightMG, file);
    fprintf(file, "const int KnightEG[64] = {\n");
    printPSQTToFile(params->TKnightEG, file);
    fprintf(file, "const int BishopMG[64] = {\n");
    printPSQTToFile(params->TBishopMG, file);
    fprintf(file, "const int BishopEG[64] = {\n");
    printPSQTToFile(params->TBishopEG, file);
    fprintf(file, "const int RookMG[64] = {\n");
    printPSQTToFile(params->TRookMG, file);
    fprintf(file, "const int RookEG[64] = {\n");
    printPSQTToFile(params->TRookEG, file);
    fprintf(file, "const int QueenMG[64] = {\n");
    printPSQTToFile(params->TQueenMG, file);
    fprintf(file, "const int QueenEG[64] = {\n");
    printPSQTToFile(params->TQueenEG, file);
    fprintf(file, "const int KingMG[64] = {\n");
    printPSQTToFile(params->TKingMG, file);
    fprintf(file, "const int KingEG[64] = {\n");
    printPSQTToFile(params->TKingEG, file);
    fprintf(file, "\n");
}

static void initParams(S_EVAL_PARAMS *params) {
	int matVals[5] = {100, 325, 325, 550, 1000,};
	memcpy(params->TPieceValMG, matVals, sizeof(matVals));
    params->TBishopPairMG = TBishopPairMG;
	memcpy(params->TPawnPassedMG, TPawnPassedMG, sizeof(TPawnPassedMG));
	memcpy(params->TPawnPassedConnectedMG, TPawnPassedConnectedMG, sizeof(TPawnPassedConnectedMG));
    params->TPawnConnectedMG = TPawnConnectedMG;
    params->TPawnIsolatedMG = TPawnIsolatedMG;
    params->TRookOpenFileMG = TRookOpenFileMG;
    params->TRookSemiOpenFileMG = TRookSemiOpenFileMG;
    params->TQueenOpenFileMG = TQueenOpenFileMG;
    params->TQueenSemiOpenFileMG = TQueenSemiOpenFileMG;
    params->TtempoMG = TtempoMG;

	memcpy(params->TPieceValEG, matVals, sizeof(matVals));
    params->TBishopPairEG = TBishopPairEG;
	memcpy(params->TPawnPassedEG, TPawnPassedEG, sizeof(TPawnPassedEG));
	memcpy(params->TPawnPassedConnectedEG, TPawnPassedConnectedEG, sizeof(TPawnPassedConnectedEG));
    params->TPawnConnectedEG = TPawnConnectedEG;
    params->TPawnIsolatedEG = TPawnIsolatedEG;
    params->TRookOpenFileEG = TRookOpenFileEG;
    params->TRookSemiOpenFileEG = TRookSemiOpenFileEG;
    params->TQueenOpenFileEG = TQueenOpenFileEG;
    params->TQueenSemiOpenFileEG = TQueenSemiOpenFileEG;
    params->TtempoEG = TtempoEG;

	memcpy(params->TTropismValues, TTropismValues, sizeof(TTropismValues));
	memcpy(params->TTropismAdjs, TTropismAdjs, sizeof(TTropismAdjs));
	memcpy(params->TTropismMatAdjs, TTropismMatAdjs, sizeof(TTropismMatAdjs));

	memcpy(params->TPawnMG, TPawnMG, sizeof(TPawnMG));
	memcpy(params->TPawnEG, TPawnEG, sizeof(TPawnEG));
	memcpy(params->TKnightMG, TKnightMG, sizeof(TKnightMG));
	memcpy(params->TKnightEG, TKnightEG, sizeof(TKnightEG));
	memcpy(params->TBishopMG, TBishopMG, sizeof(TBishopMG));
	memcpy(params->TBishopEG, TBishopEG, sizeof(TBishopEG));
	memcpy(params->TRookMG, TRookMG, sizeof(TRookMG));
	memcpy(params->TRookEG, TRookEG, sizeof(TRookEG));
	memcpy(params->TQueenMG, TQueenMG, sizeof(TQueenMG));
	memcpy(params->TQueenEG, TQueenEG, sizeof(TQueenEG));
	memcpy(params->TKingMG, TKingMG, sizeof(TKingMG));
	memcpy(params->TKingEG, TKingEG, sizeof(TKingEG));
}


static int count_lines(FILE* file)
{
    char buf[256];
    int counter = 1;
    for(;;)
    {
        size_t res = fread(buf, 1, 256, file);
        if (ferror(file))
            return -1;

        int i;
        for(i = 0; i < res; i++)
            if (buf[i] == '\n')
                counter++;

        if (feof(file))
            break;
    }
	fseek(file, 0, SEEK_SET);
    return counter;
}

void readFENScore(char fen[MAX_FEN_LEN], double *score, FILE* inputFile) {
	int character;
	int index = 0;
    while ((character = fgetc(inputFile)) != EOF && character != '[') {
        fen[index++] = (char)character;
    }
	fscanf(inputFile, "%lf]\n", score);
}

void readFENScores(char (*fenBuf)[MAX_FEN_LEN], double *scores, FILE* inputFile, int numPos) {
	for (int i = 0; i < numPos; i++) {
		readFENScore(fenBuf[i], &scores[i], inputFile);
	}
}

static double getError(S_BOARD *pos, S_EVAL_PARAMS *params, double K, char (*fenBuf)[MAX_FEN_LEN], double *scores, int numPos, int isTanh) {
	double error = 0.0;
	for (int i = 0; i < numPos; i++) {
		double result = scores[i];
		// read FEN + score in pos
		ParseFen(fenBuf[i], pos);
		// evaluate the position
		int score = EvalPositionTunable(pos, params);
		double sigmoid = 1 / (1 + pow(10, ((-K * score) / 400)));
		double tanhResult = 2 * result - 1;
		double tanh_eval = tanh((K * score) / 400);
		// add to error
		error += (isTanh) ? pow(tanhResult - tanh_eval, 2) : pow(result - sigmoid, 2);
	}
	return error / numPos;
}

static void getFinalErrors(S_BOARD *pos, S_EVAL_PARAMS *params, char (*fenBuf)[MAX_FEN_LEN], double *scores, int numPos, double *final_sig, double *final_tanh, double sig_K, double tanh_K) {
	*final_sig = getError(pos, params, sig_K, fenBuf, scores, numPos, 0);
	*final_tanh = getError(pos, params, tanh_K, fenBuf, scores, numPos, 1);
}

int getRandomNumber(int step_size, float change_likelihood) {
    srand((unsigned int)time(NULL));
    int randomNumber = rand() % (step_size * 2 + 1);
    return (((double)rand() / (double)RAND_MAX) > change_likelihood) ? randomNumber - step_size : 0;
}

// Generate a random number between 0 and 1
double random01() {
    return rand() / (double)RAND_MAX;
}

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void shuffleIndices(int *indices, int size) {
    srand((unsigned int)time(NULL));

    // Initialize indices array
    for (int i = 0; i < size; ++i) {
        indices[i] = i;
    }

	int PSQTIndex = size - (64 * 6 * 2);

	// Fisher-Yates shuffle the general indices
    for (int i = PSQTIndex; i > 0; --i) {
        int j = rand() % (i + 1);
        swap(&indices[i], &indices[j]);
    }

    // Fisher-Yates shuffle the PSQT indices
    for (int i = size - 1; i > PSQTIndex; --i) {
        int j = rand() % (i + 1);
        swap(&indices[i], &indices[j]);
    }

	// this way, the general terms are optimized before the psqt, but there is shuffling
}

void simulatedAnnealing() {

}

void TuneEval(S_BOARD *pos, char *fileIn, char *fileOut, char *fileLog, int use_tanh) {
	int adjust_val = 1;
	double K = 1;
	int useTanh = use_tanh;

	FILE *input = fopen(fileIn, "r");
	int numPos = count_lines(input);

	printf("Opened %s, with %d positions\n", fileIn, numPos);

	S_EVAL_PARAMS params[1];
	initParams(params);

	FILE* out = fopen(fileOut, "w+");
	fclose(out);

	FILE* log = fopen(fileLog, "w+");
	fclose(log);

	
	int *params_list = (int *)malloc(sizeof(S_EVAL_PARAMS));
	int *new_params_list = (int *)malloc(sizeof(S_EVAL_PARAMS));
	int params_length = sizeof(S_EVAL_PARAMS) / sizeof(params_list[0]);

	// start with current weights
	paramsToList(params, params_list);

	// read FENs + scores in
	char (*fens_buf)[MAX_FEN_LEN] = malloc(sizeof(char[numPos][MAX_FEN_LEN]));
	double *scores_buf = (double *)malloc(numPos * sizeof(double));
	readFENScores(fens_buf, scores_buf, input, numPos);
	fclose(input);

	double last_error = INFINITE;
	double best_error = getError(pos, params, K, fens_buf, scores_buf, numPos, useTanh);

	printf("Optimizing K before tuning: \n");
	double K_adj_val = 0.1;
	while (best_error < last_error) {
		last_error = best_error;
		printf("Local Search - K: %0.03lf\r", K);
		K += K_adj_val;
		listToParams(params_list, params);
		double new_error = getError(pos, params, K, fens_buf, scores_buf, numPos, useTanh);
		if (new_error < best_error) {
			best_error = new_error;
			continue;
		}
		K -= 2 * K_adj_val;
		listToParams(params_list, params);
		new_error = getError(pos, params, K, fens_buf, scores_buf, numPos, useTanh);
		if (new_error < best_error) {
			best_error = new_error;
			continue;
		}
		K += K_adj_val; // reset if failed
	}
	printf("\n");

	// show initial error
	printf("Initial error before tuning: %0.06lf\n", best_error);

	// then tune with simulated annealing (for exploration) and local search (for exploitation)
	int epoch = 0;
	double init_temp = 100;
	do {
		epoch++;
		last_error = best_error;
		printf("Epoch %d: \n", epoch);

		// first do simulated annealing
		// explorative part of the algorithm, so annealing parameters are tuned for high exploration
		int sim_iters = 400;
		double temperature = init_temp / epoch;
		double cooling_rate = 0.95;
		double tune_probability = 0.005;
		int tune_size = 2;
		printf(" - Simulated Annealing\n");
		for (int i = 0; i < sim_iters; i++) {
			for (int i = 0; i < params_length; i++) {
				new_params_list[i] = params_list[i] + getRandomNumber(tune_size, tune_probability);
			}
			listToParams(new_params_list, params);
			double new_error = getError(pos, params, K, fens_buf, scores_buf, numPos, useTanh);
			if (new_error < best_error || random01() < exp((best_error - new_error) / temperature)) {
				best_error = new_error;
				memcpy(params_list, new_params_list, sizeof(S_EVAL_PARAMS));
			}
			temperature *= cooling_rate;
			printf(" --- Iteration %3d - Temp %0.03lf - Error: %0.06lf\r", i + 1, temperature, best_error);
		}
		
		printf("\n");
		//printParams(params);

		// then perform local search, with a max step size max_in_a_row 
		// (can afford to be agressive with allowed in a row steps because of annealing and the shuffle)
		// exploitative part of the algorithm
		int steps_in_a_row = 0;
		int max_in_a_row = 20;
		printf(" - Shuffled Local Search\n");
		int *shuffled_indices = (int *)malloc(params_length * sizeof(int));
		shuffleIndices(shuffled_indices, params_length);
		for (int i = 0; i < params_length; i++) {
			printf(" --- Parameter %3d/%d ---------- Error: %0.06lf\r", i + 1, params_length, best_error);
			params_list[shuffled_indices[i]] += adjust_val;
			listToParams(params_list, params);
			double new_error = getError(pos, params, K, fens_buf, scores_buf, numPos, useTanh);
			if (new_error < best_error) {
				best_error = new_error;
				i--;
				steps_in_a_row++;
				if (steps_in_a_row == max_in_a_row) {
					i++;
					steps_in_a_row = 0;
				}
				continue;
			}
			params_list[shuffled_indices[i]] -= 2 * adjust_val;
			listToParams(params_list, params);
			new_error = getError(pos, params, K, fens_buf, scores_buf, numPos, useTanh);
			if (new_error < best_error) {
				best_error = new_error;
				i--;
				steps_in_a_row++;
				if (steps_in_a_row == max_in_a_row) {
					i++;
					steps_in_a_row = 0;
				}
				continue;
			}
			params_list[shuffled_indices[i]] += adjust_val; // reset if failed
		}
		printf("\n");
		// add to log file
		FILE* log = fopen(fileLog, "a");
		for (int i = 0; i < 28; i++)
			fprintf(log, "-");
		double final_tanh;
		double final_sig;
		// test final errors against a close to optimal K for most sets, so I can test wether tanh or sigmoid works better for minimization
		getFinalErrors(pos, params, fens_buf, scores_buf, numPos, &final_sig, &final_tanh, 1.4, 1.6);
		fprintf(log, "\nEpoch %d --- Error:\n - Tanh (K = %0.02lf): %lf\n -- Sig (K = %0.02lf): %lf\n", epoch, 1.6, final_tanh, 1.4, final_sig);
		printParamsToFile(params, log);
		fclose(log);

		// replace best in out file
		FILE* out = fopen(fileOut, "w");
		for (int i = 0; i < 28; i++)
			fprintf(out, "-");
		getFinalErrors(pos, params, fens_buf, scores_buf, numPos, &final_sig, &final_tanh, 1.4, 1.6);
		fprintf(out, "\nEpoch %d --- Error:\n - Tanh (K = %0.02lf): %lf\n -- Sig (K = %0.02lf): %lf\n", epoch, 1.6, final_tanh, 1.4, final_sig);
		printParamsToFile(params, log);
		fclose(out);

		free(shuffled_indices);
	} while (best_error < last_error);

	listToParams(params_list, params);
	printf("Error after tuning: %0.06lf --- ", getError(pos, params, K, fens_buf, scores_buf, numPos, useTanh));

	printf("Tuned Parameters in %s\n", fileOut);
	free(params_list);
	free(new_params_list);
	free(fens_buf);
	free(scores_buf);
}
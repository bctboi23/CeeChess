// parameters modifiable by tuning method
#ifndef EVAL_T_H
#define EVAL_T_H
typedef struct {
    int TPieceValMG[5];
    int TBishopPairMG;
    int TPawnPassedMG[8];
    int TPawnPassedConnectedMG[8];
    int TPawnConnectedMG;
    int TPawnIsolatedMG;
    int TRookOpenFileMG;
    int TRookSemiOpenFileMG;
    int TQueenOpenFileMG;
    int TQueenSemiOpenFileMG;
    int TtempoMG;

    int TPieceValEG[5];
    int TBishopPairEG;
    int TPawnPassedEG[8];
    int TPawnPassedConnectedEG[8];
    int TPawnConnectedEG;
    int TPawnIsolatedEG;
    int TRookOpenFileEG;
    int TRookSemiOpenFileEG;
    int TQueenOpenFileEG;
    int TQueenSemiOpenFileEG;
    int TtempoEG;

    int TTropismValues[4];
    int TTropismAdjs[4];
    int TTropismMatAdjs[13];

    int TPawnMG[64];
    int TPawnEG[64];
    int TKnightMG[64];
    int TKnightEG[64];
    int TBishopMG[64];
    int TBishopEG[64];
    int TRookMG[64];
    int TRookEG[64];
    int TQueenMG[64];
    int TQueenEG[64];
    int TKingMG[64];
    int TKingEG[64];
} S_EVAL_PARAMS;

typedef struct {
	S_EVAL_PARAMS params;
	double score;
	int rank;
} S_GENETIC_POINT;

// Game phase constants
const int TminorPhase = 1;
const int TrookPhase = 2;
const int TqueenPhase = 4;
const int TtotalPhase = TminorPhase * 8 + TrookPhase * 4 + TqueenPhase * 2;

// Midgame constants
const int TBishopPairMG = 20;
const int TPawnPassedMG[8] = { 0, 5, 10, 20, 35, 50, 75, 150 };
const int TPawnPassedConnectedMG[8] = { 0, 0, 5, 10, 20, 35, 50, 75};
const int TPawnConnectedMG = 15;
const int TPawnIsolatedMG = -10;
const int TRookOpenFile = 10;
const int TRookSemiOpenFile = 5;
const int TQueenOpenFile = 5;
const int TQueenSemiOpenFile = 3;
const int TtempoMG = 0;

// King safety constants
const int TTropismValues[4] = {1, 1, 2, 4};
const int TTropismAdjs[4] = {-5, -5, -5, -5};
const int TTropismMatAdjs[13] = {1, 2, 4, 8, 32, 64, 128, 192, 224, 230, 232, 234, 235};

// Endgame constants
const int TBishopPairEG = 35;
const int TPawnPassedEG[8] = { 0, 5, 15, 30, 50, 75, 125, 250 };
const int TPawnPassedConnectedEG[8] = { 0, 0, 5, 15, 30, 50, 75, 125};
const int TPawnConnectedEG = 50;
const int TPawnIsolatedEG = -20;
const int TtempoEG = 0;

const int TRookOpenFileMG = TRookOpenFile;
const int TRookOpenFileEG = TRookOpenFile;

const int TQueenOpenFileMG = TQueenOpenFile;
const int TQueenOpenFileEG = TQueenOpenFile;

const int TRookSemiOpenFileMG = TRookSemiOpenFile;
const int TRookSemiOpenFileEG = TRookSemiOpenFile;

const int TQueenSemiOpenFileMG = TQueenSemiOpenFile;
const int TQueenSemiOpenFileEG = TQueenSemiOpenFile;


// Piece Square Tables (by Lyudmil)
const int TPawnMG[64] =
{
	0,   0,   0,   0,   0,   0,   0,   0,
	-5,  -2,   4,   5,   5,   4,  -2,  -5,
	-4,  -2,   5,   7,   7,   5,  -2,  -4,
	-2,  -1,   9,  13,  13,   9,  -1,  -2,
	2,   4,  13,  21,  21,  13,   4,   2,
	10,  21,  25,  29,  29,  25,  21,  10,
	1,   2,   5,   9,   9,   5,   2,   1,             // Pawns 7 Rank
	0,   0,   0,   0,   0,   0,   0,   0
};

const int TPawnEG[64] =
{
	0,   0,   0,   0,   0,   0,   0,   0,
	-3,  -1,   2,   3,   3,   2,  -1,  -3,
	-2,  -1,   3,   4,   4,   3,  -1,  -2,
	-1,   0,   4,   7,   7,   4,   0,  -1,
	1,   2,   7,  11,  11,   7,   2,   1,
	5,  11,  13,  14,  14,  13,  11,   5,
	0,   1,   3,   5,   5,   3,   1,   0,    // Pawns 7 Rank
	0,   0,   0,   0,   0,   0,   0,   0
};

const int TKnightMG[64] =
{
	-31, -23, -20, -16, -16, -20, -23, -31,
	-23, -16, -12,  -8,  -8, -12, -16, -23,
	-8,  -4,   0,   8,   8,   0,  -4,  -8,
	-4,   8,  12,  16,  16,  12,   8,  -4,
	8,  16,  20,  23,  23,  20,  16,   8,
	23,  27,  31,  35,  35,  31,  27,  23,
	4,   8,  12,  16,  16,  12,   8,   4,
	4,   4,   4,   4,   4,   4,   4,   4,
};

const int TKnightEG[64] =
{
	-39, -27, -23, -20, -20, -23, -27, -39,
	-27, -20, -12,  -8,  -8, -12, -20, -27,
	-8,  -4,   0,   8,   8,   0,  -4,  -8,
	-4,   8,  12,  16,  16,  12,   8,  -4,
	8,  16,  20,  23,  23,  20,  16,   8,
	12,  23,  27,  31,  31,  27,  23,  12,
	-2,   2,   4,   8,   8,   4,   2,  -2,
	-16,  -8,  -4,  -4,  -4,  -4,  -8, -16,
};

const int TBishopMG[64] =
{
	-31, -23, -20, -16, -16, -20, -23, -31,
	-23, -16, -12,  -8,  -8, -12, -16, -23,
	-8,  -4,   0,   8,   8,   0,  -4,  -8,
	-4,   8,  12,  16,  16,  12,   8,  -4,
	8,  16,  20,  23,  23,  20,  16,   8,
	23,  27,  31,  35,  35,  31,  27,  23,
	4,   8,  12,  16,  16,  12,   8,   4,
	4,   4,   4,   4,   4,   4,   4,   4,
};

const int TBishopEG[64] =
{
	-39, -27, -23, -20, -20, -23, -27, -39,
	-27, -20, -12,  -8,  -8, -12, -20, -27,
	-8,  -4,   0,   8,   8,   0,  -4,  -8,
	-4,   8,  12,  16,  16,  12,   8,  -4,
	8,  16,  20,  23,  23,  20,  16,   8,
	12,  23,  27,  31,  31,  27,  23,  12,
	-2,   2,   4,   8,   8,   4,   2,  -2,
	-16,  -8,  -4,  -4,  -4,  -4,  -8, -16,
};

const int TRookMG[64] =
{
	-10,  -8,  -6,  -4,  -4,  -6,  -8, -10,
	-8,  -6,  -4,  -2,  -2,  -4,  -6,  -8,
	-4,  -2,   0,   4,   4,   0,  -2,  -4,
	-2,   2,   4,   8,   8,   4,   2,  -2,
	2,   4,   8,  12,  12,   8,   4,   2,
	4,   8,   12, 16,  16,  12,   8,   4,
	20,  21,   23, 23,  23,  23,  21,  20,
	18,  18,   20, 20,  20,  20,  18,  18,
};

const int TRookEG[64] =
{
	-10,  -8,  -6,  -4,  -4,  -6,  -8, -10,
	-8,  -6,  -4,  -2,  -2,  -4,  -6,  -8,
	-4,  -2,   0,   4,   4,   0,  -2,  -4,
	-2,   2,   4,   8,   8,   4,   2,  -2,
	2,   4,   8,  12,  12,   8,   4,   2,
	4,   8,  12,  16,  16,  12,   8,   4,
	20,  21,  23,  23,  23,  23,  21,  20,
	18,  18,  20,  20,  20,  20,  18,  18,
};

const int TQueenMG[64] =
{
	-23, -20, -16, -12, -12, -16, -20, -23,
	-18, -14, -12,  -8,  -8, -12, -14, -18,
	-16,  -8,   0,   8,   8,   0,  -8, -16,
	-8,   0,  12,  16,  16,  12,   0,  -8,
	4,  12,  16,  23,  23,  16,  12,   4,
	16,  23,  27,  31,  31,  27,  23,  16,
	4,  12,  16,  23,  23,  16,  12,   4,
	2,   8,  12,  12,  12,  12,   8,   2,
};

const int TQueenEG[64] =
{
	-23, -20, -16, -12, -12, -16, -20, -23,
	-18, -14, -12,  -8,  -8, -12, -14, -18,
	-16,  -8,   0,   8,   8,   0,  -8, -16,
	-8,   0,  12,  16,  16,  12,   0,  -8,
	4,  12,  16,  23,  23,  16,  12,   4,
	16,  23,  27,  31,  31,  27,  23,  16,
	4,  12,  16,  23,  23,  16,  12,   4,
	2,   8,  12,  12,  12,  12,   8,   2,
};

const int TKingMG[64] =
{
	40,  50,  30,  10,  10,  30,  50,  40,
	30,  40,  20,   0,   0,  20,  40,  30,
	10,  20,   0, -20, -20,   0,  20,  10,
	0,  10, -10, -30, -30, -10,  10,   0,
	-10,   0, -20, -40, -40, -20,   0, -10,
	-20, -10, -30, -50, -50, -30, -10, -20,
	-30, -20, -40, -60, -60, -40, -20, -30,
	-40, -30, -50, -70, -70, -50, -30, -40 ,
};

const int TKingEG[64] =
{
	-34, -30, -28, -27, -27, -28, -30, -34,
	-17, -13, -11, -10, -10, -11, -13, -17,
	-2,   2,   4,   5,   5,   4,   2,  -2,
	11,  15,  17,  18,  18,  17,  15,  11,
	22,  26,  28,  29,  29,  28,  26,  22,
	31,  34,  37,  38,  38,  37,  34,  31,
	38,  41,  44,  45,  45,  44,  41,  38,
	42,  46,  48,  50,  50,  48,  46,  42,
};

const int Tdiag_nw[64] = {
   0, 1, 2, 3, 4, 5, 6, 7,
   1, 2, 3, 4, 5, 6, 7, 8,
   2, 3, 4, 5, 6, 7, 8, 9,
   3, 4, 5, 6, 7, 8, 9,10,
   4, 5, 6, 7, 8, 9,10,11,
   5, 6, 7, 8, 9,10,11,12,
   6, 7, 8, 9,10,11,12,13,
   7, 8, 9,10,11,12,13,14
};

const int Tdiag_ne[64] = {
   7, 6, 5, 4, 3, 2, 1, 0,
   8, 7, 6, 5, 4, 3, 2, 1,
   9, 8, 7, 6, 5, 4, 3, 2,
  10, 9, 8, 7, 6, 5, 4, 3,
  11,10, 9, 8, 7, 6, 5, 4,
  12,11,10, 9, 8, 7, 6, 5,
  13,12,11,10, 9, 8, 7, 6,
  14,13,12,11,10, 9, 8, 7
};

int Tbonus_dia_distance[15] = {5, 4, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#endif
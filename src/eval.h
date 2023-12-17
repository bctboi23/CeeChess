#ifndef EVAL_H
#define EVAL_H

// OLD EVALUATION CONSTANTS >> eval-tuned.h is new constants

// Game phase constants
const int minorPhase = 1;
const int rookPhase = 2;
const int queenPhase = 4;
const int totalPhase = minorPhase * 8 + rookPhase * 4 + queenPhase * 2;

// Midgame constants
const int BishopPairMG = 20;
const int PawnPassedMG[8] = { 0, 5, 10, 20, 35, 50, 75, 150 };
const int PawnPassedConnectedMG[8] = { 0, 0, 5, 10, 20, 35, 50, 75};
const int PawnConnectedMG = 15;
const int PawnIsolatedMG = -10;
const int RookOpenFile = 10;
const int RookSemiOpenFile = 5;
const int QueenOpenFile = 5;
const int QueenSemiOpenFile = 3;
const int mobilityFactorMG = 5;
const int tempoMG = 3;

// Endgame constants
const int BishopPairEG = 35;
const int PawnPassedEG[8] = { 0, 5, 15, 30, 50, 75, 125, 250 };
const int PawnPassedConnectedEG[8] = { 0, 0, 5, 15, 30, 50, 75, 125};
const int PawnConnectedEG = 50;
const int PawnIsolatedEG = -20;
const int mobilityFactorEG = 2;
const int tempoEG = 2;

const int RookOpenFileMG = RookOpenFile;
const int RookOpenFileEG = RookOpenFile;

const int QueenOpenFileMG = QueenOpenFile;
const int QueenOpenFileEG = QueenOpenFile;

const int RookSemiOpenFileMG = RookSemiOpenFile;
const int RookSemiOpenFileEG = RookSemiOpenFile;

const int QueenSemiOpenFileMG = QueenSemiOpenFile;
const int QueenSemiOpenFileEG = QueenSemiOpenFile;


const int diag_nw[64] = {
   0, 1, 2, 3, 4, 5, 6, 7,
   1, 2, 3, 4, 5, 6, 7, 8,
   2, 3, 4, 5, 6, 7, 8, 9,
   3, 4, 5, 6, 7, 8, 9,10,
   4, 5, 6, 7, 8, 9,10,11,
   5, 6, 7, 8, 9,10,11,12,
   6, 7, 8, 9,10,11,12,13,
   7, 8, 9,10,11,12,13,14
};

const int diag_ne[64] = {
   7, 6, 5, 4, 3, 2, 1, 0,
   8, 7, 6, 5, 4, 3, 2, 1,
   9, 8, 7, 6, 5, 4, 3, 2,
  10, 9, 8, 7, 6, 5, 4, 3,
  11,10, 9, 8, 7, 6, 5, 4,
  12,11,10, 9, 8, 7, 6, 5,
  13,12,11,10, 9, 8, 7, 6,
  14,13,12,11,10, 9, 8, 7
};

int bonus_dia_distance[15] = {5, 4, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Piece Square Tables (by Lyudmil)
const int PawnMG[64] =
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

const int PawnEG[64] =
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

const int KnightMG[64] =
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

const int KnightEG[64] =
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

const int BishopMG[64] =
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

const int BishopEG[64] =
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

const int RookMG[64] =
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

const int RookEG[64] =
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

const int QueenMG[64] =
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

const int QueenEG[64] =
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

const int KingMG[64] =
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

const int KingEG[64] =
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

#endif

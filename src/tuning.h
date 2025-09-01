// parameters modifiable by tuning method
#ifndef EVAL_TUNE_H
#define EVAL_TUNE_H

#include "board.h"
#include "bitboards.h"
#include "evaluate.h"

#include <stdio.h>

// factor of compression for the float representation
#define COMPRESS_FAC 100.0

#define MAX_FEN_LEN 90

// compacted position representation
typedef struct {
    U64 piece_bbs[12]; // 0 - 11 are mapped to pce 1 - 12 via the enum (0 - 5 are wP - wK, 6 - 11 are bP - bK)
	U64 color_bbs[3]; // white is 0 black is 1 both is 2, all pieces of color
	int side;
	int material[2]; // we combine MG and EG material scores into one int, per color
	int king_sq[2];
} S_BOARD_TUNE;

// extern void TuneEval(S_BOARD *pos, char *fileIn, char *fileOut, char *fileLog, int use_tanh);
extern void TuneEvalSPSA(S_BOARD *pos, char *fileIn, char *fileOut, int input_batch_size, int use_tanh);
extern void printParamsToFile(S_EVAL_PARAMS *params, FILE *file);
extern void progressBar(char *title_text, int progress, int total);
extern void readFENScores(char (*fenBuf)[MAX_FEN_LEN], double *scores, FILE* inputFile, int numPos);

extern int count_lines(FILE* file);
extern int ParseFenTunable(char *fen, S_BOARD_TUNE *pos, S_EVAL_PARAMS *params);
extern int EvalPositionTunable(S_BOARD_TUNE *pos, S_EVAL_PARAMS *params);


#endif
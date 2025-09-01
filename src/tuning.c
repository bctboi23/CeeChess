// tuning.c

#include "board.h"
#include "tuning.h"
#include "eval-tuned.h"
#include "debug.h"
#include "misc.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
TUNING ONLY POSITION FUNCTIONS FOR USE IN FASTER TUNING MECHANISM
the smaller position representation allows for holding 20m+ positions in main memory (prev. can't hold more than ~300k)
This allows for much faster accesses (FEN conversion only needs doing once), and much faster tuning as a result (~20x speedup on large datasets)

Stats comparison on the limited vs full represntation:
> Number of positions: 1,000,000
> 57,320,000,000 Bytes - 57,320.0 MB Before (full representation)
> -- 632,000,000 Bytes ---- 632.0 MB After (small representation)
*/

int PieceType[13] = { 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, };

/* ---- BOARD SETUP FUNCTIONS ---- */

void ResetBoardTunable(S_BOARD_TUNE *pos) {

	int index = 0;

	for(index = 0; index < 12; ++index) {
		pos->piece_bbs[index] = 0ULL;
	}

	for(index = 0; index < 3; ++index) {
		pos->color_bbs[index] = 0ULL;
	}

	for(index = 0; index < 2; ++index) {
		pos->material[index] = 0;
	}
	for (int i = 0; i < 2; i++) {
		pos->king_sq[i] = 0;
	}

	pos->side = BOTH;

}

void UpdateMaterialScores(S_BOARD_TUNE *pos, S_EVAL_PARAMS *params) {

	// reset material counts
	for (int i = 0; i < 2; i++) {
		pos->material[i] = 0;
	}

	for (int i = 1; i < 13; i++) {
		int piece_scorable = PieceType[i];
		int colour = i <= 6 ? WHITE : BLACK;
		if (piece_scorable != 5) {
			int pce_count = POP_CNT(pos->piece_bbs[i - 1]);
			pos->material[colour] += pce_count * params->materialValTunable[piece_scorable];
			#ifdef DEBUG
				printf("piece: %d | num: %d | score-mg: %d | score-eg: %d  \n", i, pce_count, SCORE_MG(params->materialValTunable[piece_scorable]), SCORE_EG(params->materialValTunable[piece_scorable]));
			#endif
		}
	}
}

void UpdateListsMaterialTunable(S_BOARD_TUNE *pos, S_EVAL_PARAMS *params, int *pieceSqList) {

	int piece,sq,index,colour;

	for(index = 0; index < BRD_SQ_NUM; ++index) {
		sq = index;
		piece = pieceSqList[index];
		ASSERT(PceValidEmptyOffbrd(piece));
		if(piece!=OFFBOARD && piece!= EMPTY) {
			colour = PieceCol[piece];
			ASSERT(SideValid(colour));

			ASSERT(pos->pceNum[piece] < 10 && pos->pceNum[piece] >= 0);

			SETBIT(pos->piece_bbs[piece - 1], sq);
			SETBIT(pos->color_bbs[colour], sq); // 0 - 5 goes to 0 (white), 6 - 11 goes to 1 (black)
			SETBIT(pos->color_bbs[BOTH], sq);

			if (piece == wK)
				pos->king_sq[WHITE] = sq;
			if (piece == bK)
				pos->king_sq[BLACK] = sq;
		}
	}
}

// this function expands the compacted board struct and runs the EvalPosition function on the expanded struct
int EvalPositionTunable(S_BOARD_TUNE *pos, S_EVAL_PARAMS *params) {

	S_BOARD full_pos[1];

	// set up material score properly
	UpdateMaterialScores(pos, params);

	// set up board intrinsics
	full_pos->side = pos->side;
	full_pos->pceNum[0] = 0;
	for (int i = 1; i < 13; i++) {
		full_pos->pceNum[i] = POP_CNT(pos->piece_bbs[i - 1]);
	}
	for (int i = 0; i < 12; i++) {
		full_pos->piece_bbs[i] = pos->piece_bbs[i];
	}
	for (int i = 0; i < 2; i++) {
		full_pos->material[i] = pos->material[i];
	}
	for (int i = 0; i < 3; i++) {
		full_pos->color_bbs[i] = pos->color_bbs[i];
	}
	for (int i = 0; i < 2; i++) {
		full_pos->KingSq[i] = pos->king_sq[i];
	}

	// For tuning, instead of returning the score based on the side to move, return the score in terms of white
	full_pos->side = WHITE;
	return EvalPosition(full_pos, params);

}

/* ------------------------------- */

/* ---- FEN PARSING FUNCTIONS ---- */

int ParseFenTunable(char *fen, S_BOARD_TUNE *pos, S_EVAL_PARAMS *params) {

	ASSERT(fen!=NULL);
	ASSERT(pos!=NULL);

	int  rank = RANK_8;
    int  file = FILE_A;
    int  piece = 0;
    int  count = 0;
    int  i = 0;
	int  sq64 = 0;

	int pieces[64] = {0};

	ResetBoardTunable(pos);

	while ((rank >= RANK_1) && *fen) {
	    count = 1;
		switch (*fen) {
            case 'p': piece = bP; break;
            case 'r': piece = bR; break;
            case 'n': piece = bN; break;
            case 'b': piece = bB; break;
            case 'k': piece = bK; break;
            case 'q': piece = bQ; break;
            case 'P': piece = wP; break;
            case 'R': piece = wR; break;
            case 'N': piece = wN; break;
            case 'B': piece = wB; break;
            case 'K': piece = wK; break;
            case 'Q': piece = wQ; break;

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                piece = EMPTY;
                count = *fen - '0';
                break;

            case '/':
            case ' ':
                rank--;
                file = FILE_A;
                fen++;
                continue;

            default:
                printf("%c FEN error \n", *fen);
                return -1;
        }

		for (i = 0; i < count; i++) {
            sq64 = rank * 8 + file;
            if (piece != EMPTY) {
                pieces[sq64] = piece;
            }
			file++;
        }
		fen++;
	}

	ASSERT(*fen == 'w' || *fen == 'b');

	pos->side = (*fen == 'w') ? WHITE : BLACK;
	fen += 2;

	for (i = 0; i < 4; i++) {
        if (*fen == ' ') {
            break;
        }
		switch(*fen) {
			default: break;
        }
		fen++;
	}
	fen++;

	// ASSERT(pos->castlePerm>=0 && pos->castlePerm <= 15);

	if (*fen != '-') {
		file = fen[0] - 'a';
		rank = fen[1] - '1';

		ASSERT(file>=FILE_A && file <= FILE_H);
		ASSERT(rank>=RANK_1 && rank <= RANK_8);
    }

	UpdateListsMaterialTunable(pos, params, pieces);

	return 0;
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
		if (!(i & 131071))
			progressBar("Reading FENs", i, numPos);
	}
	progressBar("Reading FENs", numPos, numPos);
}

/* ------------------------------- */

/* ----- NEW PRINT FUNCTIONS ----- */

static void printBar(int size, char* bar) {
	for (int i = 0; i < size; i++) {
		printf("%s", bar);
	}
	printf("\n");
}

void progressBar(char *title_text, int progress, int total) {
    int barWidth = 40;
    float progressRatio = (float)progress / total;
    int progressBarLength = progressRatio * barWidth;

    printf("%s --- [", title_text);
    for (int i = 0; i < progressBarLength - 1; i++) {
        printf("=");
    }
	if (progress == total) {
		printf("=");
	} else {
    	printf(">");
	}
    for (int i = progressBarLength; i < barWidth; i++) {
        printf(" ");
    }
    printf("] %d / %d  \r", progress, total);
    fflush(stdout);
}


static void printNewPieceVals(const int *array, const char* name, FILE* file) {
	fprintf(file, "{");
	for (int i = 0; i < 13; ++i) {
		switch (i) {
		case 0:
			fprintf(file, "S(0, 0), ");
			break;
		case 6:
		case 12:
			fprintf(file, "S(50000, 50000), ");
			break;
		default:
			int score = array[(i - 1) % 6];
			fprintf(file, "S(%d, %d), ", SCORE_MG(score), SCORE_EG(score));
			break;
		}
	}
	fprintf(file, "}, // %s\n\t", name);
}

static void printNewVals(const int *array, int length, int space, const char *name, FILE* file) {
	if (length == 1) {
		fprintf(file, "S(%d, %d), // %s\n\t", SCORE_MG(array[0]), SCORE_EG(array[0]), name);
	} else {
		fprintf(file, "{");
		for (int i = 0; i < length; i++) {
			fprintf(file, "S(%d, %d), ", SCORE_MG(array[i]), SCORE_EG(array[i]));
		}
		fprintf(file, "}, // %s[%d]\n\t", name, length);
	}
}

static void printNewPSQTToFile(const int *array, const char *name, int dim, int end, FILE* file) {
	fprintf(file, "{\n\t");
    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
			int score = array[i * dim + j];
            fprintf(file, "S(%*d, %*d), ", 3, SCORE_MG(score), 3, SCORE_EG(score));
        }
        fprintf(file, "\n\t");
    }
	if (end) {
		fprintf(file, "}, // %s[%d]\n}};\n", name, dim * dim);
	}
	else {
		fprintf(file, "}, // %s[%d]\n\t", name, dim * dim);
	}
}

static void printNewParamsToFile(S_EVAL_PARAMS *params, FILE *file) {
	fprintf(file, "S_EVAL_PARAMS curr_params[1] = {{\n\t");

	// material values
	printNewPieceVals(params->materialValTunable, "PieceVal", file);
	printNewVals(params->materialValTunable, 5, 3, "materialValTunable", file);

	// pawns
	printNewVals(params->PawnIsolated, 4, 3, "PawnIsolated", file);
	printNewVals(params->PawnDoubled, 4, 3, "PawnDoubled", file);
	printNewVals(&params->PawnConnected, 1, 3, "PawnConnected", file);
	printNewVals(&params->PawnAttack, 1, 3, "PawnAttack", file);
	printNewVals(&params->PawnStorm, 1, 3, "PawnStorm", file);
	printNewVals(&params->PawnShield, 1, 3, "PawnShield", file);

	// passers
	printNewVals(params->PassedRank, 8, 3, "PassedRank", file);
	printNewVals(params->PawnCanAdvance, 8, 3, "PawnCanAdvance", file);
	printNewVals(params->PassedFile, 4, 3, "PassedFile", file);
	printNewVals(&params->PassedLeverable, 1, 3, "PassedLeverable", file);
	printNewVals(&params->SafePromotionPath, 1, 3, "SafePromotionPath", file);
	printNewVals(&params->OwnKingPawnTropism, 1, 3, "OwnKingPawnTropism", file);
	printNewVals(&params->EnemyKingPawnTropism, 1, 3, "EnemyKingPawnTropism", file);

	// knights
	printNewVals(params->KnightMobility, 9, 3, "KnightMobility", file);
	printNewVals(params->KnightInSiberia, 4, 3, "KnightInSiberia", file);
	printNewVals(&params->KnightAttacker, 1, 3, "KnightAttacker", file);
	printNewVals(&params->KnightAttack, 1, 3, "KnightAttack", file);
	printNewVals(&params->KnightCheck, 1, 3, "KnightCheck", file);
	printNewVals(&params->KnightBehindPawn, 1, 3, "KnightBehindPawn", file);

	// bishops
	printNewVals(params->BishopMobility, 14, 3, "BishopMobility", file);
	printNewVals(&params->BishopAttacker, 1, 3, "BishopAttacker", file);
	printNewVals(&params->BishopAttack, 1, 3, "BishopAttack", file);
	printNewVals(&params->BishopCheck, 1, 3, "BishopCheck", file);
	printNewVals(&params->BishopPair, 1, 3, "BishopPair", file);
	printNewVals(&params->BishopBehindPawn, 1, 3, "BishopBehindPawn", file);
	printNewVals(&params->BishopLongDiagonal, 1, 3, "BishopLongDiagonal", file);
	printNewVals(&params->BishopRammedPawns, 1, 3, "BishopRammedPawns", file);
	
	// rooks
	printNewVals(params->RookMobility, 15, 3, "RookMobility", file);
	printNewVals(&params->RookAttacker, 1, 3, "RookAttacker", file);
	printNewVals(&params->RookAttack, 1, 3, "RookAttack", file);
	printNewVals(&params->RookCheck, 1, 3, "RookCheck", file);
	printNewVals(params->RookFile, 2, 3, "RookFile", file);
	printNewVals(&params->RookOn7th, 1, 3, "RookOn7th", file);
	printNewVals(&params->RookOnQueenFile, 1, 3, "RookOnQueenFile", file);

	// queens
	printNewVals(params->QueenMobility, 28, 3, "QueenMobility", file);
	printNewVals(&params->QueenAttacker, 1, 3, "QueenAttacker", file);
	printNewVals(&params->QueenAttack, 1, 3, "QueenAttack", file);
	printNewVals(&params->QueenCheck, 1, 3, "QueenCheck", file);

	// kings
	printNewVals(params->attackerAdj, 4, 3, "attackerAdj", file);
	printNewVals(&params->MinorDefenders, 1, 3, "MinorDefenders", file);
	printNewVals(&params->NoQueen, 1, 3, "NoQueen", file);
	printNewVals(&params->WeakSquare, 1, 3, "WeakSquare", file);

	// threats
	printNewVals(&params->WeakPawn, 1, 3, "WeakPawn", file);
	printNewVals(&params->MinorAttackedByPawn, 1, 3, "MinorAttackedByPawn", file);
	printNewVals(&params->MinorAttackedByMinor, 1, 3, "MinorAttackedByMinor", file);
	printNewVals(&params->MinorAttackedByMajor, 1, 3, "MinorAttackedByMajor", file);
	printNewVals(&params->RookAttackedByLesser, 1, 3, "RookAttackedByLesser", file);
	printNewVals(&params->MinorAttackedByKing, 1, 3, "MinorAttackedByKing", file);
	printNewVals(&params->RookAttackedByKing, 1, 3, "RookAttackedByKing", file);
	printNewVals(&params->QueenAttackedByPiece, 1, 3, "QueenAttackedByPiece", file);
	printNewVals(&params->RestrictedPiece, 1, 3, "RestrictedPiece", file);
	
	// PSQTS
	printNewPSQTToFile(params->PawnPSQT, "PawnPSQT", 8, 0, file);
	printNewPSQTToFile(params->KnightPSQT, "KnightPSQT", 8, 0, file);
	printNewPSQTToFile(params->BishopPSQT, "BishopPSQT", 8, 0, file);
	printNewPSQTToFile(params->RookPSQT, "RookPSQT", 8, 0, file);
	printNewPSQTToFile(params->QueenPSQT, "QueenPSQT", 8, 0, file);
	printNewPSQTToFile(params->KingPSQT, "KingPSQT", 8, 1, file);
}

static void prettyPrintStats(FILE* file, S_EVAL_PARAMS *params, int epoch, double train_err, double valid_err) {
	fprintf(file, ".");
	for (int i = 0; i < 24; i++)
		fprintf(file, "-");
	fprintf(file, ".\n");
	// Print the padding manually
	fprintf(file, "| ---- Epoch #");
	int width = 6 - log10(epoch);
	fprintf(file, "%d ", epoch);
	for (int i = 0; i < width - 1; i++) {
		fprintf(file, "-");
	}
	fprintf(file, "---- |\n");
	fprintf(file, "| Train Error | %lf |\n| Valid Error | %lf |\n", train_err, valid_err);
	fprintf(file, "\'");
	for (int i = 0; i < 24; i++)
		fprintf(file, "-");
	fprintf(file, "\'\n\n");
	printNewParamsToFile(params, file);
}

/* ------------------------------- */

/* ------- MISC. FUNCTIONS ------- */

static void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

double lerp(double a, double b, double f) {
	return (1 - f) * a + f * b;
}

int count_lines(FILE* file) {
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

static void shuffleIndices(int *indices, int size) {
    // Initialize indices array
    for (int i = 0; i < size; ++i) {
        indices[i] = i;
    }
	// Fisher-Yates shuffle the indices
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        swap(&indices[i], &indices[j]);
    }
}

static void initParams(S_EVAL_PARAMS *params) {
	memcpy(params, curr_params, sizeof(S_EVAL_PARAMS));
}

static void paramsToFloatList(S_EVAL_PARAMS *params, int* paramsList, int* initialParamsList, double* paramsListFloat, int params_len) {
    memcpy(paramsList, params, sizeof(S_EVAL_PARAMS));
    // we divide the current parameters by some large number (100 here) to squeeze them together in our float list
	// this allows us greater precision during training, as floating points get more accurate the closer to zero they are

	// all integer params store two scores in one (midgame and endgame), so we have to split them to add them to the float list
    for (int i = 0; i < params_len; i++) {
        paramsListFloat[2 * i] = SCORE_MG(paramsList[i]) / COMPRESS_FAC;
		paramsListFloat[2 * i + 1] = SCORE_EG(paramsList[i]) / COMPRESS_FAC;
    }
}

static void FloatListToParams(S_EVAL_PARAMS *params, int* paramsList, int* initialParamsList, double* paramsListFloat, int params_len) {
    // to get back the parameter values we then multiply by that same original divide
	// all integer params store two scores in one (midgame and endgame), so we have to combine float scores back to the int list
    for (int i = 0; i < params_len; i++) {
        paramsList[i] = MAKE_SCORE((int) (paramsListFloat[2 * i] * COMPRESS_FAC), 
								   (int) (paramsListFloat[2 * i + 1] * COMPRESS_FAC));
    }
	memcpy(params, paramsList, sizeof(S_EVAL_PARAMS));
}

/* ------------------------------- */

/* ---- MAIN TUNING FUNCTIONS ---- */

double pseudoHuberLoss(double true_y, double pred_y) {
	return 4 * (sqrt(1 + (pow((true_y - pred_y), 2) / 2)) - 1);
}

double MSELoss(double true_y, double pred_y) {
	return (true_y - pred_y) * (true_y - pred_y);
}

double BCELoss(double true_y, double pred_y) {
	return -(true_y * log(pred_y) + (1 - true_y) * log((1 - pred_y)));
}

static double getError(S_BOARD_TUNE *boards, S_EVAL_PARAMS *params, double K, double *scores, int numPos, int posOffset, int *shuffled_indices, int isTanh) {
	
	double error = 0.0;
	#pragma omp parallel for reduction (+:error)
	for (int i = posOffset; i < numPos + posOffset; i++) {
        double result;
        S_BOARD_TUNE board;
        if (shuffled_indices == NULL) {
		    result = scores[i];
            board = boards[i];
        } else {
            result = scores[shuffled_indices[i]];
            board = boards[shuffled_indices[i]];
        }
		// evaluate the position
		int score = EvalPositionTunable(&board, params);
		double eval = (isTanh) ? tanh((K * score) / 400) : 1 / (1 + pow(10, ((-K * score) / 400)));
		result = (isTanh) ? 2 * result - 1 : result;
		// this is incorrect for the stockfish augmented dataset so commented out
		//double tanhResult = result;
		// add to error (error is based on pseudo-huber loss, to reduce the power of outliers)
		// pseudohuber loss
		error += pseudoHuberLoss(result, eval);
		// error += (isTanh) ? pseudoHuberLoss(tanhResult, tanh_eval): pseudoHuberLoss(result, sigmoid);
		//error += BCELoss(result, sigmoid);
		// binary crossentropy loss
		//error += -(result * log(clamped_sig) + (1 - result) * log((1 - clamped_sig)));
	}
	return error / numPos;
}

void perturbParameters(double *parameters, double *directions, double *parameters_up, double *parameters_down, int num_params, double magnitude) {
    for (int j = TUNABLE_START; j < num_params; j++) {
        directions[j] = (rand() % 2) * 2 - 1; // Bernouli random
        parameters_up[j] = parameters[j] + magnitude * directions[j];
        parameters_down[j] = parameters[j] - magnitude * directions[j];
    }
}

void TuneEvalSPSA(S_BOARD *pos, char *fileIn, char *fileOut, int input_batch_size, int use_tanh) {
	double K = 1;
	int useTanh = use_tanh;

	FILE *input = fopen(fileIn, "r");
    if (input == NULL) {
        perror("Error opening file");
        return;
    }
	int numPos = MAX(0, count_lines(input));

	printf("Opened %s, with %d positions\n", fileIn, numPos);

	S_EVAL_PARAMS params[1];
	// start with current weights
	initParams(params);

	char outFileName[256];
	strncpy(outFileName, fileOut, sizeof(&fileOut));
	strncat(outFileName, ".out", sizeof(outFileName) - strlen(outFileName) - 1);
	FILE* out = fopen(outFileName, "w+");
	printNewParamsToFile(params, out);
	fclose(out);

	char logFileName[256];
	strncpy(logFileName, fileOut, sizeof(&fileOut));
	strncat(logFileName, ".log", sizeof(logFileName) - strlen(logFileName) - 1);
	FILE* log = fopen(logFileName, "w+");
	fclose(log);


	char csvFileName[256];
	strncpy(csvFileName, fileOut, sizeof(&fileOut));
	strncat(csvFileName, ".csv", 4 * sizeof(csvFileName) - strlen(csvFileName) - 1);
	FILE *csv = fopen(csvFileName, "w+");
	fprintf(csv, "epoch,train,valid\n");
	fclose(csv);
	
	int params_length = sizeof(S_EVAL_PARAMS) / sizeof(int);
    int *params_int_list = (int *)malloc(sizeof(int) * params_length);
    int *initial_params = (int *)malloc(sizeof(int) * params_length);

	// float params lists are twice the size of the int lists (since the ints store two scores, mg and eg)
	int float_params_length = params_length * 2;
	size_t float_list_size	= sizeof(double) * float_params_length;
	
    double *params_float_list 	  = (double *)malloc(float_list_size);
    double *params_float_list_up  = (double *)malloc(float_list_size);
    double *params_float_list_lo  = (double *)malloc(float_list_size);
    double *params_gradient_list  = (double *)malloc(float_list_size);
	double *params_ewa_g_list 	  = (double *)malloc(float_list_size);
	double *params_ewa_v_list 	  = (double *)malloc(float_list_size);
    double *params_direction_list = (double *)malloc(float_list_size);

    // we first save the original parameters to condense everything to small floats and use SGD gradients
    memcpy(initial_params, params, sizeof(S_EVAL_PARAMS));

	// remove PSQT starting values
    for (int i = 0; i < 64; i++) {
		params->PawnPSQT[i] = 0;
		params->KnightPSQT[i] = 0;
		params->BishopPSQT[i] = 0;
		params->RookPSQT[i] = 0;
		params->QueenPSQT[i] = 0;
		params->KingPSQT[i] = 0;
	}

    // and intialize the float list
	paramsToFloatList(params, params_int_list, initial_params, params_float_list, params_length);
	
	// read FENs + scores in
	char (*fens_buf)[MAX_FEN_LEN] = malloc(sizeof(char[numPos][MAX_FEN_LEN]));
	double *scores_buf = (double *)malloc(numPos * sizeof(double));
    int *indices = (int *)malloc(numPos * sizeof(int));
	readFENScores(fens_buf, scores_buf, input, numPos);
	fclose(input);

	// load them into boards (we can use main memory for this because our board size is significantly smaller in the tuning phase)
	S_BOARD_TUNE *boards = malloc(sizeof(S_BOARD_TUNE) * numPos);
	for (int i = 0; i < numPos; i++) {
		ParseFenTunable(fens_buf[i], &boards[i], params);
	}
	free(fens_buf);

	// get index of training set and validation set (90 - 10 split)
	int numPosTrain = 0.9 * numPos;
	int numPosVal = numPos - numPosTrain;
	int valOffset = numPosTrain;

	float trainPercent = (float)numPosTrain / (float)numPos * 100;
	float validPercent = (float)numPosVal / (float)numPos * 100;
	int maxWidth = snprintf(NULL, 0, "%d", numPos);
	printf("\n > Train examples: %*d (%.1f%%)\n > Valid examples: %*d (%.1f%%)\n", maxWidth, numPosTrain, trainPercent, maxWidth, numPosVal, validPercent);

    FloatListToParams(params, params_int_list, initial_params, params_float_list, params_length);
	double best_error = getError(boards, params, K, scores_buf, numPosTrain, 0, NULL, useTanh);
	printf("Initial error before K optimization: %0.06lf\n", best_error);

	// first tune the K via local search
	printf("Optimizing K before tuning: \n");
	double K_adj_val = 0.1;
	double last_error = best_error;
	do {
		last_error = best_error;
		printf("Local Search - K: %0.03lf\r", K); 
		K += K_adj_val;
		double new_error = getError(boards, params, K, scores_buf, numPosTrain, 0, NULL, useTanh);
		if (new_error < best_error) {
			best_error = new_error;
			continue; 
		}
		K -= 2 * K_adj_val;
		new_error = getError(boards, params, K, scores_buf, numPosTrain, 0, NULL, useTanh);
		if (new_error < best_error) {
			best_error = new_error;
			continue;
		}
		K += K_adj_val; // reset if failed
	} while (best_error < last_error);
	printf("\n");

	// show initial error
	double init_valid_error = getError(boards, params, K, scores_buf, numPosVal, valOffset, NULL, useTanh);
	printf("Initial error before tuning: %0.06lf \\ %0.06lf\n", best_error, init_valid_error);

	// tune using SPSA
	int sgd_max = 20000;

	// we perturb at a 3:1 ratio of the compression (essentially over 3 values instead of 1) to increase gradient
	// stability, at the cost of gradient locality/precision (gradient is less accurate to the single point), 
	// but it is worth it in testing so far (2:1 is worth it, not sure about 3+:1 yet) (3:1 is better, testing 4:1)
	// essentially, we better capture the effect of parameters with "smoothish" gradients while losing fidelity
	// for "sharper" gradients. This works because our loss landscape is relatively smooth, by and large
	double perturb_magnitude = 3 / COMPRESS_FAC; 

	// more repeats gives more accurate gradients, but each repeat requires 2 function evals, so we start with 1
	// repeat, and "warm up" to max_repeats to speed up training (since earlier epochs require less gradient accuracy)
	int max_repeats = 4; 
	
	// AdaBelief parameters
	double lr = 1e-3;
	double beta_1 = 0.9;
	double beta_2 = 0.99;
	double eps = 1e-8;
	double weight_decay = 0;

	//double lr_decay = 0.95; // decay happens every 500 epochs

	double curr_err = best_error;
	double valid_error = INFINITY; 
	double best_valid = valid_error;
	int batch_size = MIN(numPosTrain / 2, input_batch_size);

    // set up initial step sizes
    for (int i = 0; i < float_params_length; i++) {
		params_ewa_g_list[i] = 0;
		params_ewa_v_list[i] = 0;
    }
    
	int start = GetTimeMs();
	printf("\n\nPerforming Integer Relaxed SPSAdaBelief (SPSA using AdaBelief parameter updates)\n");
    printBar(117, "-");

	int improved = 1;
	int iter = 0;
	for (int epoch = 1; epoch <= sgd_max;) {

		// if early in training (less than 500 epochs) don't train the PSQT values
		// int trainable_params_length = (epoch < 500) ? float_params_length - (2 * PSQT_LEN) : float_params_length;
		int trainable_params_length = float_params_length;

		// Calculate proper batching to iterate over all of the data before reshuffling
		int num_batches = numPosTrain / batch_size;
		int batch_idx = iter % num_batches;
		int offset = batch_idx * batch_size;
		int corrected_batch_size = batch_size;

		// if we are at the start of a batch reshuffle
		if (batch_idx == 0) {
			shuffleIndices(indices, numPosTrain);
		}

		// we combine the rest of the positions in the last batch
		if (batch_idx == num_batches - 1) {
			corrected_batch_size = numPosTrain - offset;
		}

		// Calculate the approximate gradient of the function via Simultaneous Perturbation. 
		// We repeat this calculation and average the results to get more accurate gradients
		int curr_repeats = MIN(max_repeats, (epoch / 1000) + 1);
		for (int i = 0; i < float_params_length; i++) {
			params_gradient_list[i] = 0;
		}
		for (int r = 0; r < curr_repeats; r++) {
			perturbParameters(params_float_list, params_direction_list, params_float_list_up, params_float_list_lo, trainable_params_length, perturb_magnitude);

			FloatListToParams(params, params_int_list, initial_params, params_float_list_up, params_length);
			double high_error = getError(boards, params, K, scores_buf, corrected_batch_size, offset, indices, useTanh);

			FloatListToParams(params, params_int_list, initial_params, params_float_list_lo, params_length);
			double low_error = getError(boards, params, K, scores_buf, corrected_batch_size, offset, indices, useTanh);

			for (int i = TUNABLE_START; i < trainable_params_length; i++) {
				params_gradient_list[i] += (high_error - low_error) / (2 * perturb_magnitude *  params_direction_list[i]) / curr_repeats; // average the calculated gradient over the measurement num
			}
		}

		// Take an optimization step using this gradient via Cautious ADAptive Belief estimation with weight decay (C-AdaBelief)
        for (int i = TUNABLE_START; i < trainable_params_length; i++) {

			// perform AdaBelief steps
			params_ewa_g_list[i] = lerp(params_gradient_list[i], params_ewa_g_list[i], beta_1);
			params_ewa_v_list[i] = lerp(pow(params_ewa_g_list[i] - params_gradient_list[i], 2) + eps, params_ewa_v_list[i], beta_2);

			double g_hat = params_ewa_g_list[i] / (1 - beta_1);
			double v_hat = params_ewa_v_list[i] / (1 - beta_2);
			
			// update parameters cautiously (don't update if gradient and update are not aligned)
			// only works better when we have more accurate gradients (repeats > 2)
			double update = g_hat / (sqrt(v_hat) + eps);
			int mask = ((curr_repeats <= 2) || update * params_gradient_list[i] > 0);
			params_float_list[i] = params_float_list[i] - mask * update * lr;

			// weight decay
			if (weight_decay != 0)
				params_float_list[i] = params_float_list[i] - mask * lr * weight_decay * params_float_list[i];
        }

		// calculate the new error
		// we only do this after a batch is complete because it is much more expensive than per batch calculations
		// this sacrifices some measuring accuracy inside batches but it is worth it for the compute speedup
		if (batch_idx == num_batches - 1) {

			FloatListToParams(params, params_int_list, initial_params, params_float_list, params_length);
			
			curr_err = getError(boards, params, K, scores_buf, numPosTrain, 0, NULL, useTanh);
			valid_error = getError(boards, params, K, scores_buf, numPosVal, valOffset, NULL, useTanh);

			FILE *csv = fopen(csvFileName, "a");
			fprintf(csv, "%d,%lf,%lf\n", epoch, curr_err, valid_error);
			fclose(csv);

			if (curr_err < best_error) {
				best_error = curr_err;
				best_valid = valid_error;
				improved = 1;
				// replace best in out file
				FILE* out = fopen(outFileName, "w");
				prettyPrintStats(out, params, epoch, best_error, valid_error);
				fclose(out);
			}

			char progress_text[256] = {0};
			if (epoch % 500 == 0) {
				// print stats
				snprintf(progress_text, 64, "Epoch: %5d | Train loss: %lf | Valid loss: %lf |", epoch, best_error, best_valid);
				progressBar(progress_text, 500, 500);
				printf("\n");
	
				// log final errors at every checkpoint
				FILE* log = fopen(logFileName, "a");
				prettyPrintStats(log, params, epoch, best_error, valid_error);
				fclose(log);		

				// if the best error hasn't improved over the last 500 iterations we stop the optimization
				if (improved == 0)
					break;
				improved = 0;
				
			} else {
				snprintf(progress_text, 64, "Epoch: %5d | This error: %lf | Best error: %lf |", epoch, curr_err, best_error);
				int progress = epoch % 500;
				progressBar(progress_text, progress, 500);
			}

			epoch++;
		}

		iter++;
	}
	int total = GetTimeMs() - start;
	int hours = total / (1000 * 60 * 60);
	int minutes = (total / (1000 * 60)) % 60;
	int seconds = (total / 1000) % 60;

	free(indices);
	printf("\n\nFinal Error: \n > Train: %0.06lf\n > Valid: %0.06lf\n", best_error, best_valid);

	printf("\nTuned by SGD in %d hours, %d minutes, and %d seconds \nParameters in %s.out\n", hours, minutes, seconds, fileOut);
    free(params_int_list);
	free(params_float_list);
    free(params_gradient_list);
	free(scores_buf);
	free(boards);
}

/* ------------------------------- */


/* OLD TUNING CODE (APSO + local search) --- DEPRECATED */
/*
void TuneEval(S_BOARD *pos, char *fileIn, char *fileOut, char *fileLog, int use_tanh) {
	double K = 1;
	int useTanh = use_tanh;

	FILE *input = fopen(fileIn, "r");
	int numPos = MAX(0, count_lines(input));

	printf("Opened %s, with %d positions\n", fileIn, numPos);

	S_EVAL_PARAMS params[1];
	// start with current weights
	initParams(params);

	FILE* out = fopen(fileOut, "w+");
	printParamsToFile(params, out);
	fclose(out);

	FILE* log = fopen(fileLog, "w+");
	fclose(log);

	char *graph_file = "graph-ce-l.csv";
	FILE *csv = fopen(graph_file, "w+");
	fprintf(csv, "epoch,best,avg\n");
	fclose(csv);

	
	int *params_list = (int *)malloc(sizeof(S_EVAL_PARAMS));
	int *new_params_list = (int *)malloc(sizeof(S_EVAL_PARAMS));
	int params_length = sizeof(S_EVAL_PARAMS) / sizeof(params_list[0]);

	paramsToList(params, params_list);
	// set all PSQT to zero before training
	for (int i = 232; i < params_length; i++) {
		params_list[i] = 0;
	}
	listToParams(params_list, params);

	// read FENs + scores in
	char (*fens_buf)[MAX_FEN_LEN] = malloc(sizeof(char[numPos][MAX_FEN_LEN]));
	double *scores_buf = (double *)malloc(numPos * sizeof(double));
	readFENScores(fens_buf, scores_buf, input, numPos);
	fclose(input);
	printf("%s\n", fens_buf[0]);

	// load them into boards (we can use main memory for this because our board size is significantly smaller in the tuning phase)
	S_BOARD_TUNE *boards = malloc(sizeof(S_BOARD_TUNE) * numPos);
	for (int i = 0; i < numPos; i++) {
		ParseFenTunable(fens_buf[i], &boards[i], params);
	}
	free(fens_buf);

	// get index of training set and validation set (90 - 10 split)
	int numPosTrain = 0.9 * numPos;
	int numPosVal = numPos - numPosTrain;
	int valOffset = numPosTrain;

	double best_error = getError(boards, params, K, scores_buf, numPosTrain, 0, useTanh);

	// 
	printf("Initial error before K optimization: %0.06lf\n", best_error);

	// first tune the K via local search
	printf("Optimizing K before tuning: \n");
	double K_adj_val = 0.1;
	double last_error = best_error;
	do {
		last_error = best_error;
		printf("Local Search - K: %0.03lf\r", K);
		K += K_adj_val;
		if (K == 0) {
			K = 0.1;
			break;
		}
		listToParams(params_list, params);
		double new_error = getError(boards, params, K, scores_buf, numPosTrain, 0, useTanh);
		if (new_error < best_error) {
			best_error = new_error;
			continue;
		}
		K -= 2 * K_adj_val;
		listToParams(params_list, params);
		new_error = getError(boards, params, K, scores_buf, numPosTrain, 0, useTanh);
		if (new_error < best_error) {
			best_error = new_error;
			continue;
		}
		K += K_adj_val; // reset if failed
	} while (best_error < last_error);
	printf("\n");

	// show initial error
	printf("Initial error before tuning: %0.06lf\n", best_error);

	// tune using A-PSO

	// set up the initial population
	// we use a flat array with access like [i][j] = [i * params_length + j]
	int (*population)[params_length] = malloc(sizeof(int[POP_SIZE][params_length]));
	S_EVAL_PARAMS pop_params[1];

	for (int i = 0; i < POP_SIZE; i++) {
		progressBar("Creating population", i + 1, POP_SIZE);
		for (int j = 26; j < params_length - 768; j++) {	
			int step_size = 2 * sqrt(POP_SIZE + params_length);
			int randomNumber = rand() % (step_size * 2 + 1) - step_size;
			population[i][j] = params_list[i] + randomNumber;
		}
	}

	// now perform the PSO loop
	int epoch;
	int pso_max = 30;
	printf("\n\nPerforming Integer Constrained A-PSO (Accelerated Particle Swarm Optimization)\n");
	for (epoch = 0; epoch < pso_max; epoch++) {
		printBar(96, "-");

		// evaluate the population and check for a new best
		double pop_avg = 0;
		double pop_best = 1000;
		for (int i = 0; i < POP_SIZE; i++) {
			progressBar("Evaluating point positions", i + 1, POP_SIZE);
			listToParams(population[i], pop_params);
			double pop_error = getError(boards, pop_params, K, scores_buf, numPosTrain, 0, useTanh);
			pop_avg += pop_error / POP_SIZE;
			if (pop_error < pop_best) {
				pop_best = pop_error;
			}
			if (pop_error < best_error) {
				best_error = pop_error;
				memcpy(params, pop_params, sizeof(S_EVAL_PARAMS));
			}
		}
		printf("\n");
		
		// calculate the new positions
		paramsToList(params, params_list);
		double alpha_decay = pow(ALPHA, (1 + (double)epoch / 100));
		for (int i = 0; i < POP_SIZE; i++) {
			progressBar("Deriving updated locations", i + 1, POP_SIZE);
			for (int j = 26; j < params_length - 768; j++) {
				double uniform_random_num = 2 * ((double) rand() / RAND_MAX) - 1;
				double current_part = (1 - BETA) * (double) population[i][j];
				double best_part = BETA * (double) params_list[j];
				double random_part = alpha_decay * params_length * uniform_random_num;
				population[i][j] = (int) (current_part + best_part + random_part);
			}
		}

		// DEPRECATED --- perform tournament selection
		/*
		for (int i = 0; i < POP_SIZE; i++) {
			progressBar("Calculating new positions", i + 1, POP_SIZE);
			int competitor_1 = rand() % POP_SIZE;
			int competitor_2 = rand() % POP_SIZE;
			while (competitor_1 == competitor_2) {
				competitor_2 = rand() % POP_SIZE;
			}
			int winner = (pop_errors[competitor_1] < pop_errors[competitor_2]) ? competitor_1 : competitor_2;
			memcpy(children[i], population[winner], sizeof(int) * params_length);
		}
		printf("\n");
		// perform mutation
		for (int i = 0; i < POP_SIZE; i++) {
			progressBar("Mutating population", i + 1, POP_SIZE);
			for (int j = 0; j < params_length; j++) {
				float randomNumber = (float) rand() / RAND_MAX;
				if (randomNumber < MUT_RATE) {
					int mut_size = round(pow(POP_SIZE, 1./4)) * (((j < 10) * 9) + 1); // mutation size is based on 4th root of population size
					int mutation = rand() % (mut_size * 2 + 1) - mut_size;
					children[i][j] = params_list[i] + mutation;
				}
			}
		}
		// move the children into the new population
		memcpy(population, children, sizeof(int[POP_SIZE][params_length]));


		printf("\n");
		printBar(96, "-");

		// print stats
		printf("Generation %d / %d:\n > Current best error: %lf\n > This generation best error: %lf\n > This generation average error: %lf\n\n", epoch + 1, pso_max, best_error, pop_best, pop_avg);

		// test final errors
		FILE* log = fopen(fileLog, "a");
		for (int i = 0; i < 28; i++)
			fprintf(log, "-");
		double train_error = getError(boards, params, K, scores_buf, numPosTrain, 0, useTanh);
		double valid_error = getError(boards, params, K, scores_buf, numPosVal, valOffset, useTanh);
		fprintf(log, "\n--- Epoch %d ---\n - Train Error: %lf\n - Valid Error: %lf\n", epoch, train_error, valid_error);
		printParamsToFile(params, log);
		fclose(log);

		// replace best in out file
		FILE* out = fopen(fileOut, "w");
		for (int i = 0; i < 28; i++)
			fprintf(out, "-");
		fprintf(log, "\n--- Epoch %d ---\n - Train Error: %lf\n - Valid Error: %lf\n", epoch, train_error, valid_error);
		printParamsToFile(params, log);
		fclose(out);

		FILE *csv = fopen(graph_file, "a");
		fprintf(csv, "%d,%lf,%lf\n", epoch, pop_best, pop_avg);
		fclose(csv);
	}
	

	// then exploit the best found from the exploration tuned APSO in local search
	int *shuffled_indices = (int *)malloc(params_length * sizeof(int));
	printf("Performing Shuffled Local Search\n");
	do {
		epoch++;
		last_error = best_error;
		printf("Epoch %d: \n", epoch);
		//printParams(params);

		// then perform local search, with a max step size max_in_a_row 
		// exploitative part of the algorithm
		int curr_adjust_val = 1;
		int best_adj_val = 0;
		shuffleIndices(shuffled_indices, params_length);
		for (int i = 0; i < params_length; i++) {
			int curr_param = params_list[shuffled_indices[i]];
			printf(" > Parameter %3d/%d --- Error: %0.06lf\r ", i + 1, params_length, best_error);
			
			
			params_list[shuffled_indices[i]] = curr_param + curr_adjust_val;
			listToParams(params_list, params);
			double new_error = getError(boards, params, K, scores_buf, numPosTrain, 0, useTanh);
			if (new_error < best_error) {
				do {
					best_error = new_error;
					best_adj_val = curr_adjust_val;
					curr_adjust_val *= 2;
					if (curr_adjust_val >= 8)
						break;
					params_list[shuffled_indices[i]] = curr_param + curr_adjust_val;
					listToParams(params_list, params);
					new_error = getError(boards, params, K, scores_buf, numPosTrain, 0, useTanh);
				} while (new_error < best_error);
			} else {
				curr_adjust_val = -1;
				params_list[shuffled_indices[i]] = curr_param + curr_adjust_val;
				listToParams(params_list, params);
				new_error = getError(boards, params, K, scores_buf, numPosTrain, 0, useTanh);
				while (new_error < best_error) {
					best_error = new_error;
					best_adj_val = curr_adjust_val;
					curr_adjust_val *= 2;
					if (curr_adjust_val <= -8)
						break;
					params_list[shuffled_indices[i]] = curr_param + curr_adjust_val;
					listToParams(params_list, params);
					new_error = getError(boards, params, K, scores_buf, numPosTrain, 0, useTanh);
				}
			}

			params_list[shuffled_indices[i]] = curr_param + best_adj_val; // reset if failed
			curr_adjust_val = 1;
			best_adj_val = 0;
		}
		
		printf("\n");
		// add to log file
		FILE* log = fopen(fileLog, "a");
		for (int i = 0; i < 28; i++)
			fprintf(log, "-");
		// test final errors
		double train_error = getError(boards, params, K, scores_buf, numPosTrain, 0, useTanh);
		double valid_error = getError(boards, params, K, scores_buf, numPosVal, valOffset, useTanh);
		printf(" > Train Error: %lf\n > Valid Error: %lf\n", train_error, valid_error);
		fprintf(log, "\n--- Epoch %d ---\n - Train Error: %lf\n - Valid Error: %lf\n", epoch, train_error, valid_error);
		printParamsToFile(params, log);
		fclose(log);

		// replace best in out file
		FILE* out = fopen(fileOut, "w");
		for (int i = 0; i < 28; i++)
			fprintf(out, "-");
		fprintf(log, "\n--- Epoch %d ---\n - Train Error: %lf\n - Valid Error: %lf\n", epoch, train_error, valid_error);
		printParamsToFile(params, log);
		fclose(out);

		// add to graph
		FILE *csv = fopen(graph_file, "a");
		fprintf(csv, "%d,%lf,NaN\n", epoch, best_error);
		fclose(csv);

	} while (best_error < last_error);
	free(shuffled_indices);

	listToParams(params_list, params);
	double train_error = getError(boards, params, K, scores_buf, numPosTrain, 0, useTanh);
	double valid_error = getError(boards, params, K, scores_buf, numPosVal, valOffset, useTanh);
	printf("Error after tuning --- Train: %0.06lf | Valid: %0.06lf", train_error, valid_error);

	printf("Tuned Parameters in %s\n", fileOut);
	free(params_list);
	free(new_params_list);
	free(scores_buf);
	free(boards);
}
*/
#ifndef EVALUATE_H
#define EVALUATE_H

#include "board.h"
#include "eval-tuned.h"

#define SCORE_MG(s) ((int16_t)((uint16_t)((unsigned)((s)))))
#define SCORE_EG(s) ((int16_t)((uint16_t)((unsigned)((s) + 0x8000) >> 16)))

#define PSQT_SQ(sq) (us == WHITE) ? sq : MIRROR64(sq)

enum {
    SCALE_OCB = 16,
    SCALE_LONE_QUEEN = 48,
    SCALE_ONE_FLANK = 56,
    SCALE_NORMAL = 64,
    SCALE_LARGE_PAWN_ADV = 72,
};

extern int DistTable[64][64];

extern int EvalPosition(const S_BOARD *pos, const S_EVAL_PARAMS *params);
extern void MirrorEvalTest(S_BOARD *pos);
extern void InitEval();

#endif
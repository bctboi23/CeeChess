#ifndef EVAL_TUNED_H
#define EVAL_TUNED_H

// the first 258 params are calculated via formula instead of changed manually --- DEPRECATED, now just the first 26
#define TUNABLE_START 26

// the length of the PSQTs
#define PSQT_LEN 64 * 6

// we convert all scores into midgame and endgame scores first
#define MAKE_SCORE(mg, eg) ((int)((unsigned int)(eg) << 16) + (mg))
#define S(mg, eg) MAKE_SCORE(mg, eg)

extern const int minorPhase;
extern const int rookPhase;
extern const int queenPhase;
extern const int totalPhase;

extern const int diag_nw[64];
extern const int diag_ne[64];
extern const int bonus_dia_distance[15];

typedef struct {
   // material
   int PieceVal[13];
   int materialValTunable[5];

   // pawns
   int PawnIsolated[4];
   int PawnDoubled[4];
   int PawnConnected;
   int PawnAttack;
   int PawnStorm;
   int PawnShield;

   int PassedRank[8];
   int PawnCanAdvance[8];
   int PawnSafeAdvance[8];
   int PassedLeverable;
   int SafePromotionPath;
   int OwnKingPawnTropism;
   int EnemyKingPawnTropism;

   // knights
   int KnightMobility[9];
   int KnightInSiberia[4];
   int KnightAttacker;
   int KnightAttack;
   int KnightCheck;
   int KnightOutpost;
   int KnightBehindPawn;

   // bishops
   int BishopMobility[14];
   int BishopAttacker;
   int BishopAttack;
   int BishopCheck;
   int BishopPair;
   int BishopBehindPawn;
   int BishopLongDiagonal;
   int BishopRammedPawns;

   // rooks
   int RookMobility[15];
   int RookAttacker;
   int RookAttack;
   int RookCheck;

   int RookFile[2];
   int RookOn7th;
   int RookOnQueenFile;

   // queens
   int QueenMobility[28];
   int QueenAttacker;
   int QueenAttack;
   int QueenCheck;

   // king attack
   int attackerAdj[4];
   int MinorDefenders;
   int NoQueen;
   int WeakSquare;

   // threats
   int WeakPawn;
   int MinorAttackedByPawn;
   int MinorAttackedByMinor;
   int MinorAttackedByMajor;
   int RookAttackedByLesser;
   int MinorAttackedByKing;
   int RookAttackedByKing;
   int QueenAttackedByPiece;
   int RestrictedPiece;

   // PSQTs
   int PawnPSQT[64];
   int KnightPSQT[64];
   int BishopPSQT[64];
   int RookPSQT[64];
   int QueenPSQT[64];
   int KingPSQT[64];

} S_EVAL_PARAMS;

extern S_EVAL_PARAMS curr_params[1];

#endif
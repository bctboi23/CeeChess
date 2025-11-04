#ifndef BOARD_H
#define BOARD_H

#include "bitboards.h"

#define BRD_SQ_NUM 64

#define MAXGAMEMOVES 2048
#define MAXPOSITIONMOVES 256
#define MAXDEPTH 64

#define NAME "CeeChess_v2.2"

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define FR2SQ(f,r) ( (f) + ( (r) * 8 ) )

#define IsBQ(p) (PieceBishopQueen[(p)])
#define IsRQ(p) (PieceRookQueen[(p)])
#define IsB(p) (PieceBishop[(p)])
#define IsR(p) (PieceRook[(p)])
#define IsKn(p) (PieceKnight[(p)])
#define IsKi(p) (PieceKing[(p)])

#define MIRROR64(sq) (sq ^ 56)

#define COL(i) ((i) % 8)
#define ROW(i) ((i) / 8)

#define RELATIVE_ROW(i, color) ((color == WHITE) ? ROW(i) : RANK_8 - ROW(i))

// --- NOTE!!! THESE MACROS ARE NOT IMMUNE TO SIDE EFFECTS! TAKE CARE NOT TO USE THEM WHEN THIS PROBLEM COULD ARISE ---
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

enum { FALSE, TRUE, ORDER };

enum { WHITE, BLACK, BOTH };

enum { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE };
enum { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE };

// this is for use in the move storage
#define NUM_PTYPES 13
enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK  };

enum { WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8 };

/*
enum {
    A1 = 21, B1, C1, D1, E1, F1, G1, H1,
    A2 = 31, B2, C2, D2, E2, F2, G2, H2,
    A3 = 41, B3, C3, D3, E3, F3, G3, H3,
    A4 = 51, B4, C4, D4, E4, F4, G4, H4,
    A5 = 61, B5, C5, D5, E5, F5, G5, H5,
    A6 = 71, B6, C6, D6, E6, F6, G6, H6,
    A7 = 81, B7, C7, D7, E7, F7, G7, H7,
    A8 = 91, B8, C8, D8, E8, F8, G8, H8, NO_SQ, OFFBOARD
};
*/

enum {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8, NO_SQ, OFFBOARD
};

typedef struct {
    U64 posKey;
	int move;
	int castlePerm;
	int enPas;
	int fiftyMove;

} S_UNDO;

typedef struct {
    S_UNDO history[MAXGAMEMOVES];

	U64 piece_bbs[12]; // 0 - 11 are mapped to pce 1 - 12 via the enum (0 - 5 are wP - wK, 6 - 11 are bP - bK)
	U64 color_bbs[3]; // white is 0 black is 1 both is 2, all pieces of color
    U64 posKey;

    int PvArray[MAXDEPTH];
    int evalStack[MAXDEPTH];
	int searchHistory[2][BRD_SQ_NUM][BRD_SQ_NUM];
	int searchKillers[2][MAXDEPTH];

    int pieces[BRD_SQ_NUM]; 

	int KingSq[2];

	int side;
	int enPas;
	int fiftyMove;

	int ply;
	int hisPly;

	int castlePerm;

	int pceNum[NUM_PTYPES];
    int bigPce[2];
	int material[2];

} S_BOARD;

extern char PceChar[];
extern char SideChar[];
extern char RankChar[];
extern char FileChar[];

extern int FilesBrd[BRD_SQ_NUM];
extern int RanksBrd[BRD_SQ_NUM];

extern int PieceCol[NUM_PTYPES];

extern int PieceBig[NUM_PTYPES];
 
extern int PiecePawn[NUM_PTYPES];
extern int PieceKnight[NUM_PTYPES];
extern int PieceKing[NUM_PTYPES];
extern int PieceBishop[NUM_PTYPES];
extern int PieceRook[NUM_PTYPES];
extern int PieceRookQueen[NUM_PTYPES];
extern int PieceBishopQueen[NUM_PTYPES];
extern int PieceSlides[NUM_PTYPES];


extern U64 PieceKeys[NUM_PTYPES][BRD_SQ_NUM];
extern U64 SideKey;
extern U64 CastleKeys[16];

extern void MirrorBoard(S_BOARD *pos);
extern void PrintBoard(const S_BOARD *pos);
extern void ResetBoard(S_BOARD *pos);
extern void UpdateListsMaterial(S_BOARD *pos);

extern int CheckBoard(const S_BOARD *pos);
extern int ParseFen(char *fen, S_BOARD *pos);

#endif
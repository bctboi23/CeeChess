// seeChess.c

#include "stdio.h"
#include "defs.h"
#include "stdlib.h"

#define PAWNMOVES "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"

int main() {

	AllInit();

	S_BOARD board[1];

	ParseFen(PAWNMOVES, board);
	PrintBoard(board);

	S_MOVELIST list[1];

	GenerateAllMoves(board, list);

	PrintMoveList(list);

	return 0;
}

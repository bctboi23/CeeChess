// seeChess.c

#include "stdio.h"
#include "defs.h"
#include "stdlib.h"

#define PERFTFEN "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"

int main() {

	AllInit();

	S_BOARD board[1];
	S_MOVELIST list[1];

	ParseFen(START_FEN, board);

	GenerateAllMoves(board, list);

	int MoveNum = 0;
	int move = 0;

	PrintBoard(board);
	getchar();

	for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
		move = list->moves[MoveNum].move;

		if (!MakeMove(board, move)) {
			continue;
		}

		printf("\nMADE:%s", PrMove(move));
		PrintBoard(board);

		TakeMove(board);
		printf("\nTAKEN:%s", PrMove(move));
		PrintBoard(board);

		getchar();
	}
	// PrintMoveList(list);

	return 0;
}

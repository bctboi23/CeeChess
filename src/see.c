// see.c

#include "stdio.h"
#include "defs.h"

/*
 Static Exchange Evaluation code
 Idea from Raven chess engine: https://github.com/sgriffin53/raven
*/

// piece attack directions
const int KnDir[8] = { -8, -19,	-21, -12, 8, 19, 21, 12 };
const int RkDir[4] = { -1, -10,	1, 10 };
const int BiDir[4] = { -9, -11, 11, 9 };
const int KiDir[8] = { -1, -10,	1, 10, -9, -11, 11, 9 };

// find and return a capture move involving the least valuable attacking piece
static int leastValuableAttacker(const int sq, const int side, const S_BOARD *pos) {

}

// evaluate the static exchange value of the position
static int SEE(const int sq, const int side, S_BOARD *pos) {

  int SEEValue = 0;
  int piece = pos->pieces[sq];
  int move = leastValuableAttacker(sq, side, pos);

  if (move) {
    MakeMove(pos, move);
    SEEValue = MAX(0, PieceVal[piece] - SEE(sq, side ^ 1, pos));
    TakeMove(pos, move);
  }

  return SEEValue;
}

// evaluate the static exchange value of a capture move
int SEECapture(int move, S_BOARD *pos) {

  int captureValue = 0;
  int capturePiece = pos->pieces[TOSQ(move)];

  MakeMove(move);
  captureValue = PieceVal[capturePiece] - SEE(TOSQ(move), side ^ 1, pos);
  TakeMove(move);

  return captureValue;
}

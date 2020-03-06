// see.c

#include "stdio.h"
#include "defs.h"

/*
 Static Exchange Evaluation code
 Idea from Raven chess engine: https://github.com/sgriffin53/raven
*/

// piece attack directions
const int KnightDir[8] = { -8, -19,	-21, -12, 8, 19, 21, 12 };
const int RookDir[4] = { -1, -10,	1, 10 };
const int BishopDir[4] = { -9, -11, 11, 9 };
const int KingDir[8] = { -1, -10,	1, 10, -9, -11, 11, 9 };

// find and return a capture move involving the least valuable attacking piece
static int leastValuableAttacker(const int sq, const int side, const S_BOARD *pos) {

  int pce,index,t_sq,dir;

  ASSERT(SqOnBoard(sq));
  ASSERT(SideValid(side));
  ASSERT(CheckBoard(pos));

  // handling pawn moves and promotions
  if(side == WHITE) {
    // white pawns
    if(pos->pieces[sq - 11] == wP) {
      if (RanksBrd[sq] == RANK_8) {
        return MOVE(sq - 11, sq, pos->pieces[sq], wQ, 0);
      }
      else {
        return MOVE(sq - 11, sq, pos->pieces[sq], EMPTY, 0);
      }
    }
    if(pos->pieces[sq - 9] == wP) {
      if (RanksBrd[sq] == RANK_8) {
        return MOVE(sq - 9, sq, pos->pieces[sq], wQ, 0);
      }
      else {
        return MOVE(sq - 9, sq, pos->pieces[sq], EMPTY, 0);
      }
    }
  } else {
    // black pawns
    if(pos->pieces[sq + 11] == bP) {
      if (RanksBrd[sq] == RANK_1) {
        return MOVE(sq + 11, sq, pos->pieces[sq], bQ, 0);
      }
      else {
        return MOVE(sq + 11, sq, pos->pieces[sq], EMPTY, 0);
      }
    }
    if(pos->pieces[sq + 9] == bP) {
      if (RanksBrd[sq] == RANK_1) {
        return MOVE(sq + 9, sq, pos->pieces[sq], bQ, 0);
      }
      else {
        return MOVE(sq + 9, sq, pos->pieces[sq], EMPTY, 0);
      }
    }
  }

  // handling knight moves
  for(index = 0; index < 8; ++index) {
		pce = pos->pieces[sq + KnightDir[index]];
		ASSERT(PceValidEmptyOffbrd(pce));
		if(pce != OFFBOARD && IsKn(pce) && PieceCol[pce]==side) {
			return MOVE(sq + KnightDir[index], sq, pos->pieces[sq], EMPTY, 0);
		}
	}

  // king moves (may remove them, makes SEE slower)
  for(index = 0; index < 8; ++index) {
    pce = pos->pieces[sq + KingDir[index]];
    ASSERT(PceValidEmptyOffbrd(pce));
    if(pce != OFFBOARD && IsKi(pce) && PieceCol[pce]==side) {
      return MOVE(sq + KingDir[index], sq, pos->pieces[sq], EMPTY, 0);
    }
  }

  // TODO: add all other piece move calculations

  return NOMOVE;
}

// evaluate the static exchange value on a square
static int SEE(const int sq, const int side, S_BOARD *pos) {

  int SEEValue = 0;
  int piece = pos->pieces[sq];
  int move = leastValuableAttacker(sq, side ^ 1, pos);

  ASSERT(PieceValid(piece));


  // check if a move and a piece to capture both exist
  if (move != NOMOVE && PieceValid(piece)) {

    // test if move is legal (not pinned)
    if (!MakeMove(pos, move)) {
      return SEEValue;
    }

    SEEValue = MAX(0, PieceVal[piece] - SEE(sq, side ^ 1, pos));
    TakeMove(pos);

  }

  return SEEValue;
}

// evaluate the static exchange value of a capture move
int SEECapture(int move, S_BOARD *pos) {

  int captureValue = 0;
  int capturedPiece = pos->pieces[TOSQ(move)];

  // test if move is legal (not pinned)
  if (!MakeMove(pos, move)) {
    return captureValue;
  }

  captureValue = PieceVal[capturedPiece] - SEE(TOSQ(move), pos->side ^ 1, pos);
  TakeMove(pos);

  // printf("Capture made, SEE Value: %d\n", captureValue);

  return captureValue;
}

// movegen.c

#include "stdio.h"
#include "defs.h"

/*

MoveGen(board, list)
  loop all pieces
   -> Slider loop each direction and add moves
    -> AddMove list->moves[list->count] = move; list->count++;
*/

#define MOVE(f, t, ca, pro, fl) ( (f) | ( (t) << 7 ) | ( (ca) << 14 ) | ( (pro) << 20 ) | (fl) )
#define SQOFFBOARD(sq) (FilesBrd[(sq)] == OFFBOARD)

void AddQuietMove(const S_BOARD *pos, int move, S_MOVELIST *list) {

  list->moves[list->count].move = move;
  list->moves[list->count].score = 0;
  list->count++;

}

void AddCaptureMove(const S_BOARD *pos, int move, S_MOVELIST *list) {

  list->moves[list->count].move = move;
  list->moves[list->count].score = 0;
  list->count++;

}
void AddEnPassantMove(const S_BOARD *pos, int move, S_MOVELIST *list) {

  list->moves[list->count].move = move;
  list->moves[list->count].score = 0;
  list->count++;

}

void AddWhitePawnCapMove( const S_BOARD *pos, const int from, const int to, const int cap, S_MOVELIST *list ) {
  if (RanksBrd[from] == RANK_7) {
    AddCaptureMove(pos, MOVE(from, to, cap, wQ, 0), list);
    AddCaptureMove(pos, MOVE(from, to, cap, wR, 0), list);
    AddCaptureMove(pos, MOVE(from, to, cap, wB, 0), list);
    AddCaptureMove(pos, MOVE(from, to, cap, wN, 0), list);
  }
  else {
    AddCaptureMove(pos, MOVE(from, to, cap, EMPTY, 0), list);
  }
}

void AddWhitePawnMove( const S_BOARD *pos, const int from, const int to, S_MOVELIST *list ) {
  if (RanksBrd[from] == RANK_7) {
    AddQuietMove(pos, MOVE(from, to, EMPTY, wQ, 0), list);
    AddQuietMove(pos, MOVE(from, to, EMPTY, wR, 0), list);
    AddQuietMove(pos, MOVE(from, to, EMPTY, wB, 0), list);
    AddQuietMove(pos, MOVE(from, to, EMPTY, wN, 0), list);
  }
  else {
    AddQuietMove(pos, MOVE(from, to, EMPTY, EMPTY, 0), list);
  }
}

void AddBlackPawnCapMove( const S_BOARD *pos, const int from, const int to, const int cap, S_MOVELIST *list ) {
  if (RanksBrd[from] == RANK_2) {
    AddCaptureMove(pos, MOVE(from, to, cap, bQ, 0), list);
    AddCaptureMove(pos, MOVE(from, to, cap, bR, 0), list);
    AddCaptureMove(pos, MOVE(from, to, cap, bB, 0), list);
    AddCaptureMove(pos, MOVE(from, to, cap, bN, 0), list);
  }
  else {
    AddCaptureMove(pos, MOVE(from, to, cap, EMPTY, 0), list);
  }
}

void AddBlackPawnMove( const S_BOARD *pos, const int from, const int to, S_MOVELIST *list ) {
  if (RanksBrd[from] == RANK_2) {
    AddQuietMove(pos, MOVE(from, to, EMPTY, bQ, 0), list);
    AddQuietMove(pos, MOVE(from, to, EMPTY, bR, 0), list);
    AddQuietMove(pos, MOVE(from, to, EMPTY, bB, 0), list);
    AddQuietMove(pos, MOVE(from, to, EMPTY, bN, 0), list);
  }
  else {
    AddQuietMove(pos, MOVE(from, to, EMPTY, EMPTY, 0), list);
  }
}

void GenerateAllMoves(const S_BOARD *pos, S_MOVELIST *list) {

  ASSERT(CheckBoard(pos));

  list->count = 0;

  int pce = EMPTY;
  int side = pos->side;
  int sq = 0; int t_sq = 0;
  int pceNum = 0;

  if (side == WHITE) {
    for (pceNum = 0; pceNum < pos->pceNum[wP]; ++pceNum) {
      sq = pos->pList[wP][pceNum];
      ASSERT(SqOnBoard(sq));

      if (pos->pieces[sq + 10] == EMPTY) {
        AddWhitePawnMove(pos, sq, sq + 10, list);
        if (RanksBrd[sq] == RANK_2 && pos->pieces[sq + 20] == EMPTY) {
          AddQuietMove(pos, MOVE(sq, (sq + 20), EMPTY, EMPTY, MFLAGPS), list);
        }
      }

      if (!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK) {
        AddWhitePawnCapMove(pos, sq, sq + 9, pos->pieces[sq + 9], list);
      }
      if (!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK) {
        AddWhitePawnCapMove(pos, sq, sq + 11, pos->pieces[sq + 11], list);
      }

      if (sq + 9 == pos->enPas) {
        AddCaptureMove(pos, MOVE(sq, sq + 9, EMPTY, EMPTY, MFLAGEP), list);
      }
      if (sq + 11 == pos->enPas) {
        AddCaptureMove(pos, MOVE(sq, sq + 11, EMPTY, EMPTY, MFLAGEP), list);
      }
    }
  }
  else {
    for (pceNum = 0; pceNum < pos->pceNum[bP]; ++pceNum) {
      sq = pos->pList[bP][pceNum];
      ASSERT(SqOnBoard(sq));

      if (pos->pieces[sq - 10] == EMPTY) {
        AddBlackPawnMove(pos, sq, sq - 10, list);
        if (RanksBrd[sq] == RANK_7 && pos->pieces[sq - 20] == EMPTY) {
          AddQuietMove(pos, MOVE(sq, (sq - 20), EMPTY, EMPTY, MFLAGPS), list);
        }
      }

      if (!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE) {
        AddBlackPawnCapMove(pos, sq, sq - 9, pos->pieces[sq - 9], list);
      }
      if (!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE) {
        AddBlackPawnCapMove(pos, sq, sq - 11, pos->pieces[sq - 11], list);
      }

      if (sq - 9 == pos->enPas) {
        AddCaptureMove(pos, MOVE(sq, sq - 9, EMPTY, EMPTY, MFLAGEP), list);
      }
      if (sq - 11 == pos->enPas) {
        AddCaptureMove(pos, MOVE(sq, sq - 11, EMPTY, EMPTY, MFLAGEP), list);
      }
    }
  }
}

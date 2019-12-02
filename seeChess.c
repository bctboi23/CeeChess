#include "stdio.h"
#include "stdlib.h"
#include "defs.h"

int main() {

  AllInit();
  
  int pieceOne = rand();
  int pieceTwo = rand();
  int pieceThree = rand();
  int pieceFour = rand();
  
  printf("Piece 1:%X\n", pieceOne);
  printf("Piece 2:%X\n", pieceTwo);
  printf("Piece 3:%X\n", pieceThree);
  printf("Piece 4:%X\n", pieceFour);
  
  int Key = PieceOne ^ PieceTwo ^ PieceThree ^ PieceFour;
  int TempKey = PieceOne;
  TempKey ^= PieceTwo;
  TempKey ^= PieceThree;
  TempKey ^= PieceFour;
  
  printf("Key:%X\n",Key);
  printf("TempKey:%X\n",TempKey);
  
  return 0;
}

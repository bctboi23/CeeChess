#include "stdio.h"
#include "defs.h"

int main() {

  AllInit();

  U64 playBitBoard = 0ULL;

  playBitBoard |= (1ULL << SQ64(D2));
  playBitBoard |= (1ULL << SQ64(D3));
  playBitBoard |= (1ULL << SQ64(D4));

  int sq64 = 0;

  while (playBitBoard) {
    sq64 = POP(&playBitBoard);
    printf("popped:%d\n",sq64);
    PrintBitBoard(playBitBoard);
  }

  return 0;
}

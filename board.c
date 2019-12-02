#include "stdio.h"
#include "defs.h"

void ResetBoard (S_BOARD, *pos) {

  int index = 0;
  
  for (index = 0; index < BRD_SQ_NUM; ++index) {
    pos->pieces[index] = OFFBOARD;
  }
  
  for (index = 0; index < 64; ++index) {
    pos->pieces[SQ120(index)] = EMPTY;
  }
  
}

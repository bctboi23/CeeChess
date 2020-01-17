#ifndef SEARCH_H
#define SEARCH_H

// ALl search constants

// Null Move Pruning Values
const int R = 2;
const int minDepth = 3;

// Razoring Values
const int RazorDepth = 3;
const int RazorMargin[4] = {0, 200, 400, 600};

// Reverse Futility Values
const int RevFutilityDepth = 3;
const int RevFutilityMargin[4] = {0, 350, 500, 950};

// LMR Values
const int LateMoveDepth = 3;
const int FullSearchMoves = 4;

#endif

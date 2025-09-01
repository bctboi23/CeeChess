#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "board.h"
#include "bitboards.h"

#define MAX_HASH 1024

enum {  HFNONE, HFALPHA, HFBETA, HFEXACT};

typedef struct {
	U64 posKey;
	int move;
	int score;
	int depth;
	int flags;
	int age;
} S_HASHENTRY;

typedef struct {
	S_HASHENTRY *pTable;
	int numEntries;
	int newWrite;
	int overWrite;
	int hit;
	int cut;
	int currentage;
} S_HASHTABLE;

extern S_HASHTABLE HashTable[1];

extern void ClearHashTable(S_HASHTABLE *table);
extern void InitHashTable(S_HASHTABLE *table, const int MB);
extern void StoreHashEntry(S_BOARD *pos, S_HASHTABLE *table, const int move, int score, const int flags, const int depth);

extern int GetPvLine(const int depth, S_BOARD *pos, S_HASHTABLE *table);
extern int ProbeHashEntry(S_BOARD *pos, S_HASHTABLE *table, int *move, int *score, int *tt_depth, int alpha, int beta, int depth);
extern int ProbePvMove(const S_BOARD *pos, S_HASHTABLE *table);

#endif
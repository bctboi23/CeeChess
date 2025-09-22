// init.c
#include "attack.h"
#include "bitboards.h"
#include "board.h"
#include "movegen.h"
#include "evaluate.h"
#include "debug.h"
#include "search.h"

#include <stdlib.h>
#include <stdio.h>

#define RAND_64 	((U64)rand() | \
					(U64)rand() << 15 | \
					(U64)rand() << 30 | \
					(U64)rand() << 45 | \
					((U64)rand() & 0xf) << 60 )

//int Sq120ToSq64[BRD_SQ_NUM];
//int Sq64ToSq120[64];

U64 SetMask[64];
U64 ClearMask[64];

U64 PieceKeys[13][BRD_SQ_NUM];
U64 SideKey;
U64 CastleKeys[16];

int FilesBrd[BRD_SQ_NUM];
int RanksBrd[BRD_SQ_NUM];

U64 FileBBMask[8];
U64 RankBBMask[8];

U64 AdjacentFilesMask[8];

U64 ForwardRanksMasks[2][8];

U64 BlackPassedMask[64];
U64 WhitePassedMask[64];
U64 IsolatedMask[64];

U64 PassedMask[2][64];

U64 BlackConnectedMask[64];
U64 WhiteConnectedMask[64];

U64 ConnectedMask[2][64];

U64 BlackPawnShield[64]; // unused currently, may be reworked to use later
U64 WhitePawnShield[64];

U64 KingAreaMasks[2][64];
U64 PawnShieldMasks[2][64];

void InitEvalMasks() {

	int sq, tsq, r, f;

	for(sq = 0; sq < 8; ++sq) {
        FileBBMask[sq] = 0ULL;
		RankBBMask[sq] = 0ULL;
	}

	for(r = RANK_8; r >= RANK_1; r--) {
        for (f = FILE_A; f <= FILE_H; f++) {
            sq = r * 8 + f;
            FileBBMask[f] |= (1ULL << sq);
            RankBBMask[r] |= (1ULL << sq);
        }
	}

	for (f = FILE_A; f <= FILE_H; f++) {
		AdjacentFilesMask[f] = (f == FILE_A) ? 0ull : FileBBMask[f - 1];
		AdjacentFilesMask[f] |= (f == FILE_H) ? 0ull : FileBBMask[f + 1];
	}

	for(sq = 0; sq < 64; ++sq) {
		IsolatedMask[sq] = 0ULL;
		WhitePassedMask[sq] = 0ULL;
		BlackPassedMask[sq] = 0ULL;
		PassedMask[0][sq] = 0ULL;
		PassedMask[1][sq] = 0ULL;
		WhiteConnectedMask[sq] = 0ULL;
		BlackConnectedMask[sq] = 0ULL;
		ConnectedMask[0][sq] = 0ULL;
		ConnectedMask[1][sq] = 0ULL;
		WhitePawnShield[sq] = 0ULL;
		BlackPawnShield[sq] = 0ULL;
    }

	for(sq = 0; sq < 64; ++sq) {
		tsq = sq + 8;

		if (tsq < 64) {
			WhitePawnShield[sq] |= (1ULL << tsq);
		}

        while(tsq < 64) {
            WhitePassedMask[sq] |= (1ULL << tsq);
            tsq += 8;
        }

        tsq = sq - 8;

		if (tsq >= 0) {
			BlackPawnShield[sq] |= (1ULL << tsq);
		}

        while(tsq >= 0) {
            BlackPassedMask[sq] |= (1ULL << tsq);
            tsq -= 8;
        }

        if(FilesBrd[sq] > FILE_A) {
            IsolatedMask[sq] |= FileBBMask[FilesBrd[sq] - 1];

            tsq = sq + 7;
			WhiteConnectedMask[sq] |= (1ULL << tsq);
			WhitePawnShield[sq] |= (1ULL << tsq);
            while(tsq < 64) {
                WhitePassedMask[sq] |= (1ULL << tsq);
                tsq += 8;
            }

            tsq = sq - 9;
			BlackConnectedMask[sq] |= (1ULL << tsq);
			BlackPawnShield[sq] |= (1ULL << tsq);
            while(tsq >= 0) {
                BlackPassedMask[sq] |= (1ULL << tsq);
                tsq -= 8;
            }
        }

        if(FilesBrd[sq] < FILE_H) {
            IsolatedMask[sq] |= FileBBMask[FilesBrd[sq] + 1];

            tsq = sq + 9;
			WhiteConnectedMask[sq] |= (1ULL << tsq);
			WhitePawnShield[sq] |= (1ULL << tsq);
            while(tsq < 64) {
                WhitePassedMask[sq] |= (1ULL << tsq);
                tsq += 8;
            }

            tsq = sq - 7;
			BlackConnectedMask[sq] |= (1ULL << tsq);
			BlackPawnShield[sq] |= (1ULL << tsq);
            while(tsq >= 0) {
                BlackPassedMask[sq] |= (1ULL << tsq);
                tsq -= 8;
            }
        }

		ConnectedMask[WHITE][sq] = WhiteConnectedMask[sq];
		ConnectedMask[BLACK][sq] = BlackConnectedMask[sq];
		PassedMask[WHITE][sq] = WhitePassedMask[sq];
		PassedMask[BLACK][sq] = BlackPassedMask[sq];
	}

	for (int rank = 0; rank < 8; rank++) {
        for (int i = rank + 1; i < 8; i++)
            ForwardRanksMasks[WHITE][rank] |= RankBBMask[i];
        ForwardRanksMasks[BLACK][rank] = ~ForwardRanksMasks[WHITE][rank] & ~RankBBMask[rank];
    }

	for (int sq = 0; sq < BRD_SQ_NUM; sq++) {

        KingAreaMasks[WHITE][sq] = nonSliderMoveTable[1][sq] | (1ull << sq) | (nonSliderMoveTable[1][sq] << 8);
        KingAreaMasks[BLACK][sq] = nonSliderMoveTable[1][sq] | (1ull << sq) | (nonSliderMoveTable[1][sq] >> 8);

		// if on A or H file, extend the mask one file (from Ethereal)
        KingAreaMasks[WHITE][sq] |= FileBBMask[COL(sq)] != FILE_A ? 0ull : KingAreaMasks[WHITE][sq] << 1;
		KingAreaMasks[BLACK][sq] |= FileBBMask[COL(sq)] != FILE_A ? 0ull : KingAreaMasks[BLACK][sq] << 1;

        KingAreaMasks[WHITE][sq] |= FileBBMask[COL(sq)] != FILE_H ? 0ull : KingAreaMasks[WHITE][sq] >> 1;
        KingAreaMasks[BLACK][sq] |= FileBBMask[COL(sq)] != FILE_H ? 0ull : KingAreaMasks[BLACK][sq] >> 1;

		U64 pawnShieldBB = (nonSliderMoveTable[1][sq] | (1ull << sq));
		PawnShieldMasks[WHITE][sq] = pawnShieldBB & (pawnShieldBB << 8);
		PawnShieldMasks[BLACK][sq] = pawnShieldBB & (pawnShieldBB >> 8);
    }
}

void InitFilesRanksBrd() {

	int index = 0;
	int file = FILE_A;
	int rank = RANK_1;
	int sq = A1;

	for(index = 0; index < BRD_SQ_NUM; ++index) {
		FilesBrd[index] = OFFBOARD;
		RanksBrd[index] = OFFBOARD;
	}

	for(rank = RANK_1; rank <= RANK_8; ++rank) {
		for(file = FILE_A; file <= FILE_H; ++file) {
			sq = FR2SQ(file,rank);
			FilesBrd[sq] = file;
			RanksBrd[sq] = rank;
		}
	}
}

void InitHashKeys() {

	int index = 0;
	int index2 = 0;
	for(index = 0; index < 13; ++index) {
		for(index2 = 0; index2 < BRD_SQ_NUM; ++index2) {
			PieceKeys[index][index2] = RAND_64;
		}
	}
	SideKey = RAND_64;
	for(index = 0; index < 16; ++index) {
		CastleKeys[index] = RAND_64;
	}

}

void InitBitMasks() {
	int index = 0;

	for(index = 0; index < 64; index++) {
		SetMask[index] = 0ULL;
		ClearMask[index] = 0ULL;
	}

	for(index = 0; index < 64; index++) {
		SetMask[index] |= (1ULL << index);
		ClearMask[index] = ~SetMask[index];
	}
}

/*
void InitSq120To64() {

	int index = 0;
	int file = FILE_A;
	int rank = RANK_1;
	int sq = A1;
	int sq64 = 0;
	for(index = 0; index < BRD_SQ_NUM; ++index) {
		Sq120ToSq64[index] = 65;
	}

	for(index = 0; index < 64; ++index) {
		Sq64ToSq120[index] = 120;
	}

	for(rank = RANK_1; rank <= RANK_8; ++rank) {
		for(file = FILE_A; file <= FILE_H; ++file) {
			sq = FR2SQ(file,rank);
			ASSERT(SqOnBoard(sq));
			Sq64ToSq120[sq64] = sq;
			Sq120ToSq64[sq] = sq64;
			sq64++;
		}
	}
}
	*/

void AllInit() {
	//InitSq120To64();
	InitBitMasks();
	InitHashKeys();
	InitFilesRanksBrd();
	InitNonSlideMoveLookup();
	InitEvalMasks();
	InitMvvLva();
	InitEval();
	InitPawnAttackLookup();
	for (int i = 0; i < 2; i++) {
		initMasks(i);
		initAttackTable(i);
	}
	InitSearch();
}

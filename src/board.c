// board.c

#include <stdio.h>

#include "board.h"
#include "bitboards.h"
#include "move.h"
#include "validate.h"
#include "eval-tuned.h"
#include "debug.h"

int PieceCol[13] = { BOTH, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
	BLACK, BLACK, BLACK, BLACK, BLACK, BLACK };

int PieceBig[13] = { FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, FALSE };
 
int PiecePawn[13] = { FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE };
int PieceKnight[13] = { FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE };
int PieceKing[13] = { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE };
int PieceBishop[13] = { FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE };
int PieceRook[13] = { FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE };
int PieceRookQueen[13] = { FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE };
int PieceBishopQueen[13] = { FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE };
int PieceSlides[13] = { FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE }; 

#ifdef DEBUG
static int PceListOk(const S_BOARD *pos);

int PceListOk(const S_BOARD *pos) {
	int pce = wP;
	int sq;
	int num;
	for(pce = wP; pce <= bK; ++pce) {
		if(pos->pceNum[pce]<0 || pos->pceNum[pce]>=10) return FALSE;
	}

	if(pos->pceNum[wK]!=1 || pos->pceNum[bK]!=1) return FALSE;

	for(pce = wP; pce <= bK; ++pce) {
		for(num = 0; num < pos->pceNum[pce]; ++num) {
			//sq = pos->pList[pce][num];
			if(!SqOnBoard(sq)) return FALSE;
		}
	}
    return TRUE;
}

int CheckBoard(const S_BOARD *pos) {

	int t_pceNum[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int t_bigPce[2] = { 0, 0};
	int t_material[4] = { 0, 0, 0, 0};

	int sq64,t_piece,colour;

	// check piece count and other counters
	for(sq64 = 0; sq64 < 64; ++sq64) {
		t_piece = pos->pieces[sq64];
		t_pceNum[t_piece]++;
		colour = PieceCol[t_piece];
		if( PieceBig[t_piece] == TRUE) t_bigPce[colour]++;

		t_material[colour] += curr_params->PieceVal[t_piece];
	}

	for(t_piece = wP; t_piece <= bK; ++t_piece) {
		ASSERT(t_pceNum[t_piece]==pos->pceNum[t_piece]);
	}

	// check bitboards count
	for (t_piece = wP; t_piece <= bK; ++t_piece) {
		int pcount = POP_CNT(pos->piece_bbs[t_piece - 1]);
		ASSERT(pcount==pos->pceNum[t_piece]);
	}

	// check bitboards squares
	for (t_piece = wP; t_piece <= bK; ++t_piece) {
		U64 pces = pos->piece_bbs[t_piece - 1];
		while(pces) {
			sq64 = POP_LSB(&pces);
			ASSERT(pos->pieces[sq64] == t_piece);
		}
	}

	ASSERT(t_material[WHITE]==pos->material[WHITE] && t_material[BLACK]==pos->material[BLACK]);
	ASSERT(t_material[WHITE + 2]==pos->material[WHITE + 2] && t_material[BLACK + 2]==pos->material[BLACK + 2]);
	ASSERT(t_bigPce[WHITE]==pos->bigPce[WHITE] && t_bigPce[BLACK]==pos->bigPce[BLACK]);

	ASSERT(pos->side==WHITE || pos->side==BLACK);
	ASSERT(GeneratePosKey(pos)==pos->posKey);

	ASSERT(pos->enPas==NO_SQ || ( RanksBrd[pos->enPas]==RANK_6 && pos->side == WHITE)
		 || ( RanksBrd[pos->enPas]==RANK_3 && pos->side == BLACK));

	ASSERT(pos->pieces[pos->KingSq[WHITE]] == wK);
	ASSERT(pos->pieces[pos->KingSq[BLACK]] == bK);

	ASSERT(pos->castlePerm >= 0 && pos->castlePerm <= 15);

	ASSERT(PceListOk(pos));

	return TRUE;
}
#endif

U64 GeneratePosKey(const S_BOARD *pos) {

	int sq = 0;
	U64 finalKey = 0;
	int piece = EMPTY;
	
	// pieces
	for(sq = 0; sq < BRD_SQ_NUM; ++sq) {
		piece = pos->pieces[sq];
		if(piece!=NO_SQ && piece!=EMPTY && piece != OFFBOARD) {
			ASSERT(piece>=wP && piece<=bK);
			finalKey ^= PieceKeys[piece][sq];
		}		
	}
	
	if(pos->side == WHITE) {
		finalKey ^= SideKey;
	}
		
	if(pos->enPas != NO_SQ) {
		ASSERT(pos->enPas>=0 && pos->enPas<BRD_SQ_NUM);
		ASSERT(SqOnBoard(pos->enPas));
		ASSERT(RanksBrd[pos->enPas] == RANK_3 || RanksBrd[pos->enPas] == RANK_6);
		finalKey ^= PieceKeys[EMPTY][pos->enPas];
	}
	
	ASSERT(pos->castlePerm>=0 && pos->castlePerm<=15);
	
	finalKey ^= CastleKeys[pos->castlePerm];
	
	return finalKey;
}


void PrintBoard(const S_BOARD *pos) {

	int sq,file,rank,piece;

	printf("\nGame Board:\n\n");

	for(rank = RANK_8; rank >= RANK_1; rank--) {
		printf("%d  ",rank + 1);
		for(file = FILE_A; file <= FILE_H; file++) {
			sq = FR2SQ(file, rank);
			piece = pos->pieces[sq];
			printf("%3c",PceChar[piece]);
		}
		printf("\n");
	}

	printf("\n   ");
	for(file = FILE_A; file <= FILE_H; file++) {
		printf("%3c",'a'+file);
	}
	printf("\n");
	printf("side:%c\n",SideChar[pos->side]);
	printf("enPas:%d\n",pos->enPas);
	printf("castle:%c%c%c%c\n",
			pos->castlePerm & WKCA ? 'K' : '-',
			pos->castlePerm & WQCA ? 'Q' : '-',
			pos->castlePerm & BKCA ? 'k' : '-',
			pos->castlePerm & BQCA ? 'q' : '-'
			);
	printf("PosKey:%llX\n",pos->posKey);
}

void UpdateListsMaterial(S_BOARD *pos) {

	int piece,sq,index,colour;

	pos->material[WHITE] = 0;
	pos->material[BLACK] = 0;

	for(index = 0; index < BRD_SQ_NUM; ++index) {
		sq = index;
		piece = pos->pieces[index];
		ASSERT(PceValidEmptyOffbrd(piece));
		if(piece!=OFFBOARD && piece!= EMPTY) {
			colour = PieceCol[piece];
			ASSERT(SideValid(colour));

		    if( PieceBig[piece] == TRUE) pos->bigPce[colour]++;

			pos->material[colour] += curr_params->PieceVal[piece];

			ASSERT(pos->pceNum[piece] < 10 && pos->pceNum[piece] >= 0);

			//pos->pList[piece][pos->pceNum[piece]] = sq;
			pos->pceNum[piece]++;


			if(piece==wK) pos->KingSq[WHITE] = sq;
			if(piece==bK) pos->KingSq[BLACK] = sq;

			SETBIT(pos->piece_bbs[piece - 1], sq);
			SETBIT(pos->color_bbs[colour], sq); // 0 - 5 goes to 0 (white), 6 - 11 goes to 1 (black)
		}
	}
	pos->color_bbs[BOTH] = pos->color_bbs[WHITE] | pos->color_bbs[BLACK];
}

int ParseFen(char *fen, S_BOARD *pos) {

	ASSERT(fen!=NULL);
	ASSERT(pos!=NULL);

	int  rank = RANK_8;
    int  file = FILE_A;
    int  piece = 0;
    int  count = 0;
    int  i = 0;
	int  sq64 = 0;

	ResetBoard(pos);

	while ((rank >= RANK_1) && *fen) {
	    count = 1;
		switch (*fen) {
            case 'p': piece = bP; break;
            case 'r': piece = bR; break;
            case 'n': piece = bN; break;
            case 'b': piece = bB; break;
            case 'k': piece = bK; break;
            case 'q': piece = bQ; break;
            case 'P': piece = wP; break;
            case 'R': piece = wR; break;
            case 'N': piece = wN; break;
            case 'B': piece = wB; break;
            case 'K': piece = wK; break;
            case 'Q': piece = wQ; break;

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                piece = EMPTY;
                count = *fen - '0';
                break;

            case '/':
            case ' ':
                rank--;
                file = FILE_A;
                fen++;
                continue;

            default:
                printf("FEN error \n");
                return -1;
        }

		for (i = 0; i < count; i++) {
            sq64 = rank * 8 + file;
            if (piece != EMPTY) {
                pos->pieces[sq64] = piece;
            }
			file++;
        }
		fen++;
	}

	ASSERT(*fen == 'w' || *fen == 'b');

	pos->side = (*fen == 'w') ? WHITE : BLACK;
	fen += 2;

	for (i = 0; i < 4; i++) {
        if (*fen == ' ') {
            break;
        }
		switch(*fen) {
			case 'K': pos->castlePerm |= WKCA; break;
			case 'Q': pos->castlePerm |= WQCA; break;
			case 'k': pos->castlePerm |= BKCA; break;
			case 'q': pos->castlePerm |= BQCA; break;
			default:	     break;
        }
		fen++;
	}
	fen++;

	ASSERT(pos->castlePerm>=0 && pos->castlePerm <= 15);

	if (*fen != '-') {
		file = fen[0] - 'a';
		rank = fen[1] - '1';

		ASSERT(file>=FILE_A && file <= FILE_H);
		ASSERT(rank>=RANK_1 && rank <= RANK_8);

		pos->enPas = FR2SQ(file, rank);
    }

	pos->posKey = GeneratePosKey(pos);

	UpdateListsMaterial(pos);

	return 0;
}

void ResetBoard(S_BOARD *pos) {

	int index = 0;

	for (index = 0; index < 12; ++index) {
		pos->piece_bbs[index] = 0ULL;
	}

	for (index = 0; index < 3; ++index) {
		pos->color_bbs[index] = 0ULL;
	}

	for(index = 0; index < BRD_SQ_NUM; ++index) {
		pos->pieces[index] = OFFBOARD;
	}

	for(index = 0; index < 64; ++index) {
		pos->pieces[index] = EMPTY;
	}

	for(index = 0; index < 2; ++index) {
		pos->bigPce[index] = 0;
	}

	for(index = 0; index < 2; ++index) {
		pos->material[index] = 0;
	}

	for(index = 0; index < 13; ++index) {
		pos->pceNum[index] = 0;
	}

	pos->KingSq[WHITE] = pos->KingSq[BLACK] = NO_SQ;

	pos->side = BOTH; 
	pos->enPas = NO_SQ;
	pos->fiftyMove = 0;

	pos->ply = 0;
	pos->hisPly = 0;

	pos->castlePerm = 0;

	pos->posKey = 0ULL;

}

void MirrorBoard(S_BOARD *pos) {

    int tempPiecesArray[64];
    int tempSide = pos->side^1;
	int SwapPiece[13] = { EMPTY, bP, bN, bB, bR, bQ, bK, wP, wN, wB, wR, wQ, wK };
    int tempCastlePerm = 0;
    int tempEnPas = NO_SQ;

	int sq;
	int tp;

    if (pos->castlePerm & WKCA) tempCastlePerm |= BKCA;
    if (pos->castlePerm & WQCA) tempCastlePerm |= BQCA;

    if (pos->castlePerm & BKCA) tempCastlePerm |= WKCA;
    if (pos->castlePerm & BQCA) tempCastlePerm |= WQCA;

	if (pos->enPas != NO_SQ)  {
        tempEnPas = MIRROR64(pos->enPas); 
    }

    for (sq = 0; sq < 64; sq++) {
        tempPiecesArray[sq] = pos->pieces[MIRROR64(sq)];
    }

    ResetBoard(pos);

	for (sq = 0; sq < 64; sq++) {
        tp = SwapPiece[tempPiecesArray[sq]];
        pos->pieces[sq] = tp;
    }

	pos->side = tempSide;
    pos->castlePerm = tempCastlePerm;
    pos->enPas = tempEnPas;

    pos->posKey = GeneratePosKey(pos);

	UpdateListsMaterial(pos);

    ASSERT(CheckBoard(pos));
}
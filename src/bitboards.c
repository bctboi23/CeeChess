
#include "bitboards.h"

#include <stdio.h>

static U64 attacks_table[LOOKUP_TABLE_SIZE] = {0};

typedef struct {
    U64 magic;
    U64 mask;
    int index;
} S_MAGICENTRY;

// compacted white magics from Volker Annuss
// note that masks are initialized at zero, and thus need to be generated at startup
static S_MAGICENTRY bishop_magics[64] = {
    { 0x007fbfbfbfbfbfffu,   0,   5378 },
    { 0x0000a060401007fcu,   0,   4093 },
    { 0x0001004008020000u,   0,   4314 },
    { 0x0000806004000000u,   0,   6587 },
    { 0x0000100400000000u,   0,   6491 },
    { 0x000021c100b20000u,   0,   6330 },
    { 0x0000040041008000u,   0,   5609 },
    { 0x00000fb0203fff80u,   0,  22236 },
    { 0x0000040100401004u,   0,   6106 },
    { 0x0000020080200802u,   0,   5625 },
    { 0x0000004010202000u,   0,  16785 },
    { 0x0000008060040000u,   0,  16817 },
    { 0x0000004402000000u,   0,   6842 },
    { 0x0000000801008000u,   0,   7003 },
    { 0x000007efe0bfff80u,   0,   4197 },
    { 0x0000000820820020u,   0,   7356 },
    { 0x0000400080808080u,   0,   4602 },
    { 0x00021f0100400808u,   0,   4538 },
    { 0x00018000c06f3fffu,   0,  29531 },
    { 0x0000258200801000u,   0,  45393 },
    { 0x0000240080840000u,   0,  12420 },
    { 0x000018000c03fff8u,   0,  15763 },
    { 0x00000a5840208020u,   0,   5050 },
    { 0x0000020008208020u,   0,   4346 },
    { 0x0000804000810100u,   0,   6074 },
    { 0x0001011900802008u,   0,   7866 },
    { 0x0000804000810100u,   0,  32139 },
    { 0x000100403c0403ffu,   0,  57673 },
    { 0x00078402a8802000u,   0,  55365 },
    { 0x0000101000804400u,   0,  15818 },
    { 0x0000080800104100u,   0,   5562 },
    { 0x00004004c0082008u,   0,   6390 },
    { 0x0001010120008020u,   0,   7930 },
    { 0x000080809a004010u,   0,  13329 },
    { 0x0007fefe08810010u,   0,   7170 },
    { 0x0003ff0f833fc080u,   0,  27267 },
    { 0x007fe08019003042u,   0,  53787 },
    { 0x003fffefea003000u,   0,   5097 },
    { 0x0000101010002080u,   0,   6643 },
    { 0x0000802005080804u,   0,   6138 },
    { 0x0000808080a80040u,   0,   7418 },
    { 0x0000104100200040u,   0,   7898 },
    { 0x0003ffdf7f833fc0u,   0,  42012 },
    { 0x0000008840450020u,   0,  57350 },
    { 0x00007ffc80180030u,   0,  22813 },
    { 0x007fffdd80140028u,   0,  56693 },
    { 0x00020080200a0004u,   0,   5818 },
    { 0x0000101010100020u,   0,   7098 },
    { 0x0007ffdfc1805000u,   0,   4451 },
    { 0x0003ffefe0c02200u,   0,   4709 },
    { 0x0000000820806000u,   0,   4794 },
    { 0x0000000008403000u,   0,  13364 },
    { 0x0000000100202000u,   0,   4570 },
    { 0x0000004040802000u,   0,   4282 },
    { 0x0004010040100400u,   0,  14964 },
    { 0x00006020601803f4u,   0,   4026 },
    { 0x0003ffdfdfc28048u,   0,   4826 },
    { 0x0000000820820020u,   0,   7354 },
    { 0x0000000008208060u,   0,   4848 },
    { 0x0000000000808020u,   0,  15946 },
    { 0x0000000001002020u,   0,  14932 },
    { 0x0000000401002008u,   0,  16588 },
    { 0x0000004040404040u,   0,   6905 },
    { 0x007fff9fdf7ff813u,   0,  16076 } 
};

static S_MAGICENTRY rook_magics[64] = {
    { 0x00280077ffebfffeu,   0,  26304 },
    { 0x2004010201097fffu,   0,  35520 },
    { 0x0010020010053fffu,   0,  38592 },
    { 0x0040040008004002u,   0,   8026 },
    { 0x7fd00441ffffd003u,   0,  22196 },
    { 0x4020008887dffffeu,   0,  80870 },
    { 0x004000888847ffffu,   0,  76747 },
    { 0x006800fbff75fffdu,   0,  30400 },
    { 0x000028010113ffffu,   0,  11115 },
    { 0x0020040201fcffffu,   0,  18205 },
    { 0x007fe80042ffffe8u,   0,  53577 },
    { 0x00001800217fffe8u,   0,  62724 },
    { 0x00001800073fffe8u,   0,  34282 },
    { 0x00001800e05fffe8u,   0,  29196 },
    { 0x00001800602fffe8u,   0,  23806 },
    { 0x000030002fffffa0u,   0,  49481 },
    { 0x00300018010bffffu,   0,   2410 },
    { 0x0003000c0085fffbu,   0,  36498 },
    { 0x0004000802010008u,   0,  24478 },
    { 0x0004002020020004u,   0,  10074 },
    { 0x0001002002002001u,   0,  79315 },
    { 0x0001001000801040u,   0,  51779 },
    { 0x0000004040008001u,   0,  13586 },
    { 0x0000006800cdfff4u,   0,  19323 },
    { 0x0040200010080010u,   0,  70612 },
    { 0x0000080010040010u,   0,  83652 },
    { 0x0004010008020008u,   0,  63110 },
    { 0x0000040020200200u,   0,  34496 },
    { 0x0002008010100100u,   0,  84966 },
    { 0x0000008020010020u,   0,  54341 },
    { 0x0000008020200040u,   0,  60421 },
    { 0x0000820020004020u,   0,  86402 },
    { 0x00fffd1800300030u,   0,  50245 },
    { 0x007fff7fbfd40020u,   0,  76622 },
    { 0x003fffbd00180018u,   0,  84676 },
    { 0x001fffde80180018u,   0,  78757 },
    { 0x000fffe0bfe80018u,   0,  37346 },
    { 0x0001000080202001u,   0,    370 },
    { 0x0003fffbff980180u,   0,  42182 },
    { 0x0001fffdff9000e0u,   0,  45385 },
    { 0x00fffefeebffd800u,   0,  61659 },
    { 0x007ffff7ffc01400u,   0,  12790 },
    { 0x003fffbfe4ffe800u,   0,  16762 },
    { 0x001ffff01fc03000u,   0,      0 },
    { 0x000fffe7f8bfe800u,   0,  38380 },
    { 0x0007ffdfdf3ff808u,   0,  11098 },
    { 0x0003fff85fffa804u,   0,  21803 },
    { 0x0001fffd75ffa802u,   0,  39189 },
    { 0x00ffffd7ffebffd8u,   0,  58628 },
    { 0x007fff75ff7fbfd8u,   0,  44116 },
    { 0x003fff863fbf7fd8u,   0,  78357 },
    { 0x001fffbfdfd7ffd8u,   0,  44481 },
    { 0x000ffff810280028u,   0,  64134 },
    { 0x0007ffd7f7feffd8u,   0,  41759 },
    { 0x0003fffc0c480048u,   0,   1394 },
    { 0x0001ffffafd7ffd8u,   0,  40910 },
    { 0x00ffffe4ffdfa3bau,   0,  66516 },
    { 0x007fffef7ff3d3dau,   0,   3897 },
    { 0x003fffbfdfeff7fau,   0,   3930 },
    { 0x001fffeff7fbfc22u,   0,  72934 },
    { 0x0000020408001001u,   0,  72662 },
    { 0x0007fffeffff77fdu,   0,  56325 },
    { 0x0003ffffbf7dfeecu,   0,  66501 },
    { 0x0001ffff9dffa333u,   0,  14826 } 
};

U64 nonSliderMoveTable[2][64];
U64 pawnAttackTable[2][64];

// for use in knight move and pawn move masking
const U64 not_a_file = 18374403900871474942ULL;
const U64 not_h_file = 9187201950435737471ULL;
const U64 not_ab_file = 18229723555195321596ULL;
const U64 not_hg_file = 4557430888798830399ULL;

static U64 maskBishopAttacks(int square);
static U64 maskRookAttacks(int square);

static U64 onTheFlyBishopAttacks(int square, U64 blockers);
static U64 onTheFlyRookAttacks(int square, U64 blockers);
static U64 setOccupancy(int index, int bits_in_mask, U64 attack_mask);

int inline popLSB(U64 *bb) {
	int index = __builtin_ctzll(*bb);
	*bb &= *bb - 1;
	return index;
}
  
void printBitBoard(U64 bb) {
  
      U64 shiftMe = 1ULL;
      
      printf("\n");
      for(int rank = 7; rank >= 0; --rank) {
          for(int file = 0; file <= 7; ++file) {
              int sq = 8 * rank + file;	// 64 based
              
              if((shiftMe << sq) & bb) 
                  printf("X");
              else 
                  printf("-");
                  
          }
          printf("\n");
      }  
      printf("\n\n");
  }

// mask bishop attacks
U64 maskBishopAttacks(int sq) {

    U64 attacks = 0ULL;
    int r, f;
    
    // init target rank & files
    int tr = sq / 8;
    int tf = sq % 8;
    
    // mask relevant bishop occupancy bits
    for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) attacks |= (1ULL << (r * 8 + f));
    for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) attacks |= (1ULL << (r * 8 + f));
    for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) attacks |= (1ULL << (r * 8 + f));
    for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) attacks |= (1ULL << (r * 8 + f));
    
    // return attack map
    return attacks;
}

// mask rook attacks
U64 maskRookAttacks(int sq) {

    U64 attacks = 0ULL;
    int r, f;
    
    // init target rank & files
    int tr = sq / 8;
    int tf = sq % 8;
    
    // mask relevant rook occupancy bits
    for (r = tr + 1; r <= 6; r++) attacks |= (1ULL << (r * 8 + tf));
    for (r = tr - 1; r >= 1; r--) attacks |= (1ULL << (r * 8 + tf));
    for (f = tf + 1; f <= 6; f++) attacks |= (1ULL << (tr * 8 + f));
    for (f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 8 + f));
    
    // return attack map
    return attacks;
}

// generate bishop attacks on the fly
U64 onTheFlyBishopAttacks(int square, U64 blockers) {

    U64 attacks = 0ULL;
    int r, f;
    
    // init target rank & files
    int tr = square / 8;
    int tf = square % 8;
    
    // generate bishop atacks
    for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & blockers) break;
    }
    
    for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & blockers) break;
    }
    
    for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & blockers) break;
    }
    
    for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & blockers) break;
    }
    
    // return attack map
    return attacks;
}

// generate rook attacks on the fly
U64 onTheFlyRookAttacks(int square, U64 blockers) {

    U64 attacks = 0ULL;
    int r, f;
    
    // init target rank & files
    int tr = square / 8;
    int tf = square % 8;
    
    // generate rook attacks
    for (r = tr + 1; r <= 7; r++)
    {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & blockers) break;
    }
    
    for (r = tr - 1; r >= 0; r--)
    {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & blockers) break;
    }
    
    for (f = tf + 1; f <= 7; f++)
    {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & blockers) break;
    }
    
    for (f = tf - 1; f >= 0; f--)
    {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & blockers) break;
    }
    
    // return attack map
    return attacks;
}

U64 setOccupancy(int index, int bits_in_mask, U64 attack_mask) {
    // occupancy map
    U64 occupancy = 0ULL;
    U64 mask = attack_mask;
    
    // loop over the range of bits within attack mask
    for (int count = 0; count < bits_in_mask; count++)
    {
        // pop LS1B in attack map
        int square = POP_LSB(&mask);
        
        // make sure occupancy is on board
        if (index & (1 << count))
            // populate occupancy map
            occupancy |= (1ULL << square);
    }
    
    // return occupancy map
    return occupancy;
}


void initMasks(int bishop) {
    for (int sq = 0; sq < 64; sq++) {
        if (bishop) {
            bishop_magics[sq].mask = maskBishopAttacks(sq);
        } else {
            rook_magics[sq].mask = maskRookAttacks(sq);
        }
    }
}

void initAttackTable(int bishop) {
    for (int square = 0; square < 64; square++) {

        U64 attack_mask = bishop ? bishop_magics[square].mask : rook_magics[square].mask;
        int relevant_bits_count = POP_CNT(attack_mask);
        int occupancy_indices = (1 << relevant_bits_count);

        for (int index = 0; index < occupancy_indices; index++) {
            if (bishop) {
                U64 occupancy = setOccupancy(index, relevant_bits_count, attack_mask);
                int magic_index = (occupancy * bishop_magics[square].magic) >> (64 - 9);
                attacks_table[bishop_magics[square].index + magic_index] = onTheFlyBishopAttacks(square, occupancy);
            } else {
                U64 occupancy = setOccupancy(index, relevant_bits_count, attack_mask);
                int magic_index = (occupancy * rook_magics[square].magic) >> (64 - 12);
                attacks_table[rook_magics[square].index + magic_index] = onTheFlyRookAttacks(square, occupancy);
            }
        }
    }
}

// generate pawn attacks
void InitPawnAttackLookup() {
	for (int side = 0; side <= 1; side++) {
		for (int sq = 0; sq < 64; ++sq) {
			// generate pawn attacks
			U64 attacks = 0ULL;
			U64 bitboard = 0ULL;
			SETBIT(bitboard, sq);
			
			// white pawns move up, black pawns move down, so we have to mask the a and h file differently for them
			if (!side) {
				attacks |= (bitboard << 9) & not_a_file;
				attacks |= (bitboard << 7) & not_h_file;
			} else {				
				attacks |= (bitboard >> 9) & not_h_file;
				attacks |= (bitboard >> 7) & not_a_file;    
			}
			pawnAttackTable[side][sq] = attacks;
		}
	}
}

void InitNonSlideMoveLookup() {

	// Knight move lookup generation
	for (int sq = 0; sq < 64; ++sq) {
		U64 attacks = 0;

		// piece bitboard
        U64 bitboard = 0ULL;
    
        // set piece on board
        SETBIT(bitboard, sq);
        
        // generate knight attacks
        attacks |= (bitboard << 17) & not_a_file;
        attacks |= (bitboard >> 17) & not_h_file;

        attacks |= (bitboard >> 15) & not_a_file;
        attacks |= (bitboard << 15) & not_h_file;

        attacks |= (bitboard << 10) & not_ab_file;
        attacks |= (bitboard >> 10) & not_hg_file;

        attacks |= (bitboard >> 6) & not_ab_file;
        attacks |= (bitboard << 6) & not_hg_file;

        nonSliderMoveTable[0][sq] = attacks;
	}

	// King move lookup generation
	for (int sq = 0; sq < 64; ++sq) {
        U64 attacks = 0;

		// piece bitboard
        U64 bitboard = 0ULL;
        
        // set piece on board
        SETBIT(bitboard, sq);
        
        // generate king attacks
        attacks |= (bitboard >> 8);
        attacks |= (bitboard << 8);

        attacks |= (bitboard << 9) & not_a_file;
        attacks |= (bitboard >> 9) & not_h_file;

        attacks |= (bitboard >> 7) & not_a_file;
        attacks |= (bitboard << 7) & not_h_file;

        attacks |= (bitboard << 1) & not_a_file;
        attacks |= (bitboard >> 1) & not_h_file;
        
        // put into attack map
        nonSliderMoveTable[1][sq] = attacks;
    }
}

inline U64 getBishopAttacks(int square, U64 blockers) {
    blockers &= bishop_magics[square].mask;
    blockers *= bishop_magics[square].magic;
    blockers >>= 64 - 9; // fixed shift for bishops is 9
    return attacks_table[bishop_magics[square].index + blockers];
}

inline U64 getRookAttacks(int square, U64 blockers) {
    blockers &= rook_magics[square].mask;
    blockers *= rook_magics[square].magic;
    blockers >>= 64 - 12; // fixed shift for rooks is 12
    return attacks_table[rook_magics[square].index + blockers];
}

inline U64 getAllPawnAttacks(U64 pawns, int side) {
    U64 attacks = 0ULL;
    if (!side) {
        attacks |= (pawns << 9) & not_a_file;
        attacks |= (pawns << 7) & not_h_file;
    } else {				
        attacks |= (pawns >> 9) & not_h_file;
        attacks |= (pawns >> 7) & not_a_file;    
    }
    return attacks;
}

inline U64 getAllPawnDoubleAttacks(U64 pawns, int side) {
    U64 attacks = 0ULL;
    if (!side) {
        attacks |= (pawns << 9) & not_a_file;
        attacks &= (pawns << 7) & not_h_file;
    } else {				 
        attacks |= (pawns >> 9) & not_h_file;
        attacks &= (pawns >> 7) & not_a_file;    
    }
    return attacks;
}

inline U64 singlePawnPush(U64 pawns, U64 all_pieces, int color) {
    return ( (pawns << 8) >> (color << 4) ) & ~all_pieces;
}

inline int testBit(uint64_t bb, int i) {
    return (bb & (1ull << i)) > 0;
}

inline int several(uint64_t bb) {
    return bb & (bb - 1);
}

inline int onlyOne(uint64_t bb) {
    return bb && !several(bb);
}

// perft.c

#include <stdio.h>
#include <string.h>

#include "perft.h"
#include "move.h"
#include "movegen.h"
#include "io.h"
#include "debug.h"
#include "misc.h"

#define MAX_LINE 256
#define MAX_DEPTH 6


unsigned long long leafNodes;

void Perft(int depth, S_BOARD *pos) {

    ASSERT(CheckBoard(pos));  

	if(depth == 0) {
        leafNodes++;
        return;
    }	

    S_MOVELIST list[1];
    GenerateAllMoves(pos,list);
      
    int MoveNum = 0;
	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {	
       
        if ( !MakeMove(pos,list->moves[MoveNum].move))  {
            continue;
        }
        Perft(depth - 1, pos);
        TakeMove(pos);
    }

    return;
}


void PerftTest(int depth, S_BOARD *pos) {

    ASSERT(CheckBoard(pos));

	PrintBoard(pos);
	printf("\nStarting Test To Depth:%d\n",depth);	
	leafNodes = 0;
	int start = GetTimeMs();
    S_MOVELIST list[1];
    GenerateAllMoves(pos,list);	
    
    int move;	    
    int MoveNum = 0;
	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        move = list->moves[MoveNum].move;
        if ( !MakeMove(pos,move))  {
            continue;
        }
        long cumnodes = leafNodes;
        Perft(depth - 1, pos);
        TakeMove(pos);        
        long oldnodes = leafNodes - cumnodes;
        printf("move %d : %s : %ld\n",MoveNum+1,PrMove(move),oldnodes);
    }
	int totalTime = GetTimeMs() - start;
    double knps = leafNodes / totalTime;
	printf("\nTest Complete : %lld nodes visited in %dms (%gknps)\n",leafNodes, totalTime, knps);

    return;
}

int PerftTestEPD(char *epd_file_path, S_BOARD *pos) {


    FILE *epd = fopen(epd_file_path, "r");
    if (epd == NULL) {
        perror("fopen");
        return 1;
    }
    printf("opened %s, running PERFT testing\n", epd_file_path);

    char line[MAX_LINE];

    int num_fens = 0;
    int start_time_epd = GetTimeMs();
    while (fgets(line, sizeof line, epd)) {
        num_fens++;
        /* remove trailing newline */
        line[strcspn(line, "\r\n")] = '\0';

        /* split at first ';' – everything before is the FEN   */
        char *p = strchr(line, ';');
        if (!p) continue;             /* no depth section on this line */

        *p++ = '\0';                  /* terminate FEN string */

        /* parse depth/value pairs  */
        int depth[MAX_DEPTH];
        unsigned long long value[MAX_DEPTH];
        int n = 0;

        int min_file_depth = MAX_DEPTH;

        while (*p && n < MAX_DEPTH) {
            if (sscanf(p, "D%d %lld", &depth[n], &value[n]) == 2)
                ++n;
            p = strchr(p, ';');
            if (!p) {
                min_file_depth = n;
                break;
            } 
            ++p;                       /* skip ';' */
        }

        int all_depths_passed = 1;
        ParseFen(line, pos);
        int start_time_fen = GetTimeMs();
        for (int i = 0; i < n; i++) {
            leafNodes = 0;
            Perft(i + 1, pos);
            
            if (leafNodes != value[i]) {
                if (all_depths_passed != 0)
                    printf("FEN %3d | FAILED at depth %d/%d\n", num_fens, i + 1, min_file_depth);
                all_depths_passed = 0;
                printf("FAILURE: %s depth %d - expected %lld, got %lld\n", line, depth[i], value[i], leafNodes); 
            } 
            if (all_depths_passed) {
                if (i + 1 == n)
                    printf("FEN %3d | Completed depth %d/%d ", num_fens, i + 1, min_file_depth);
                else
                    printf("FEN %3d | Completed depth %d/%d \r", num_fens, i + 1, min_file_depth);
            }
        }
        int time_elapsed_fen = GetTimeMs() - start_time_fen;
        if (all_depths_passed) {
            printf("| PASSED - %.2fs\n", (float)time_elapsed_fen / 1000.0);
        }

    }
    int time_elapsed_epd = GetTimeMs() - start_time_epd;
    printf("\nTest done in %.2fs", (float)time_elapsed_epd / 1000.0);
    fclose(epd);

    return 0;
}













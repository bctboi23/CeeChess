all:
	gcc xboard.c seeChess.c uci.c evaluate.c eval.h pvtable.c init.c bitboards.c hashkeys.c board.c data.c attack.c io.c movegen.c validate.c makemove.c perft.c search.c search.h misc.c -o seeChess -O3 -s

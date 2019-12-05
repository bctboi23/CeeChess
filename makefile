all:
	gcc seeChess.c init.c bitboards.c hashkeys.c board.c data.c attack.c io.c movegen.c validate.c -o seeChess

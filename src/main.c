// seeChess.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "hashtable.h"
#include "uci.h"
#include "console.h"
#include "init.h"

int main() {

	AllInit();

	S_BOARD pos[1];
    S_SEARCHINFO info[1];
    info->quit = FALSE;
	HashTable->pTable = NULL;
    InitHashTable(HashTable, 4);
	setbuf(stdin, NULL);
    setbuf(stdout, NULL);

	printf("Welcome to CeeChess! Type 'CeeChess' for console mode...\n");

	char line[256];
	while (TRUE) {
		memset(&line[0], 0, sizeof(line));

		fflush(stdout);

		if (!fgets(line, 256, stdin))
			continue;
		if (line[0] == '\n')
			continue;
		if (!strncmp(line, "uci",3)) {
			Uci_Loop(pos, info, HashTable);
			if(info->quit == TRUE) break;
			continue;
		} else if (!strncmp(line, "CeeChess",8))	{
			Console_Loop(pos, info, HashTable);
			if(info->quit == TRUE) break;
			continue;
		} else if(!strncmp(line, "quit",4))	{
			break;
		}
	}

	free(HashTable->pTable);

	return 0;
}
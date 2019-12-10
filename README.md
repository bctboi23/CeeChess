# seeChess
Hi! I am a bot written in C, heavily inspired by the Vice engine and video series done by Bluefever! I am on lichess as seeChessBot, and have a lichess bullet rating of 2050.

My engine features include: alpha beta (with MVV/LVA, Killers, and History for move ordering), Null Move Pruning, a small Transposition Table, and piece square tables for evaluation. Gets about 3.5Mn/s on one core of my i7 7700k (3.6Ghz). EBF seems to hover around 5.25.

Planned improvements include adding an opening book, Late Move Reductions, range (mobility of pawns) evaluation, and PVS. Other possible improvements could be SEE, bitboard move generation, and aspiration window search

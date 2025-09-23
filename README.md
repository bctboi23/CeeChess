# CeeChess
Hi! I am a bot written in C, heavily inspired by the Vice engine and video series done by Bluefever! If you want to try your hand at facing me, I am occasionally on lichess at https://lichess.org/@/seeChessBot! Play me here on fly.io: https://cee-chess.fly.dev/

**Rating:**
The rating for the latest release of the engine (v2.0), scores ~150 elo better in self-play to v1.4, and should play at ~2550-2600 CCRL (since self-play typically inflates ratings). This compares roughly to FIDE 2600, although there is no real 1-1 correspondence between these rating systems.

See the current CCRL rating here: https://www.computerchess.org.uk/ccrl/4040/cgi/engine_details.cgi?match_length=30&print=Details&each_game=0&eng=CeeChess%201.4%2064-bit#CeeChess_1_4_64-bit

Self play ratings for all versions, anchored at SeeChess 1.0 (2.0 is the newest version):   
```
Rank      Name             Elo
 1    CeeChess-v2.0    :  ~2600
 1    CeeChess-v1.4    :  ~2480
 2    CeeChess-v1.3.2  :  ~2330
 3    CeeChess-v1.3.2  :  ~2330
 4    CeeChess-v1.3.1  :  ~2315
 5    CeeChess-v1.3    :  ~2310
 6    SeeChess-v1.2    :  ~2200
 7    SeeChess-v1.1.3  :  ~2180
 8    SeeChess-v1.1.2  :  ~2165
 9    SeeChess-v1.1.1  :  ~2150
10    SeeChess-v1.1    :  ~2140
11    SeeChess-v1.0    :  ~2060
```
Most recent gauntlet with an assortment of engines:   
Time Control: (1 min, 0.5sec inc), with elo centered around the v1.4 release (ratings from bayeselo):
| Rank | Name                      | Elo  |  +  |  -  | Games | Score | Oppo. | Draws |
|------|---------------------------|------|-----|-----|-------|-------|-------|-------|
|   1  | Barbarossa-0.6.0         | 38  |  34 |  33 |  240  |  55%  |   95  |  23%  |
|   2  | CeeChess-v1.4    |  0  |  13 |  13 | 1664  |  65%  |  -13  |  26%  |
|   3  | Barbarossa-0.5.0-win10-64|  -34  |  33 |  33 |  240  |  45%  |   95  |  28%  |
|   4  | Kingfisher.v1.1.1        | -107  |  32 |  33 |  240  |  34%  |   95  |  36%  |
|   5  | gopher_check             | -146  |  34 |  35 |  238  |  29%  |   95  |  26%  |
|   6  | CeeChess 1.3.2           | -149  |  34 |  36 |  238  |  29%  |   95  |  25%  |
   ...
   
Since CCRL ratings got adjusted down recently (stockfish went from 3900 CCRL to ~3630 afaik), this no longer breaks the CCRL 2400 barrier, but comparing the results here to the old ratings of Barbarossa-0.6.0(2468), Barbarossa-0.5.0(~2375ish i believe?) and the others suggests that this release would have broken that barrier. I now expect the engine to land in the range of 2300-2350, given Barbarossa-0.6.0 has a new rating of 2355

**Next Steps:**
After the v2.0 release, I will focus on updating the search. Once that is done, I will finally move on to NNUE

# Engine Features

**Search:**
The Engine searches with a Principal Variation Search inside a Negamax framework

**Lossless Pruning:**
- Alpha-Beta pruning
- Mate Distance pruning

**Lossy Pruning:**
- Transposition Table
- Razoring
- Null Move Pruning
- Late Move Reductions
- Futility Pruning
- Static Null Move Pruning (Reverse Futility Pruning)

**Move Ordering:**
- PV Move
- Captures ordered by MVV/LVA (Most Valuable Victim/Least Valuable Attacker)
- 2 Killer Moves
- Quiet moves ordered by history heuristic

**Evaluation:**
- Material
- PSQT
- Mobility
- King Safety
- Threats
- Passed Pawns
- Knight Outposts
- Bishop pair heuristic (for Midgame and Endgame)
- Passed Pawn evaluation (Midgame and Endgame tables)
- Isolated pawn heuristic
- Open file heuristics (for Rook and Queen)
- Tapered evaluation
- Logistic Regression Tuning (Texel method) using SPSA with AdaBelief as the optimization method

**Planned Improvements (ordered by perceived feasibility):**
- Syzygy Tablebases
- SEE (Static Exchange Evaluation) (bitboards would be nice for this, but maybe feasible to do quickly in mailbox)

**Other Possible Improvements (No particular order):**
- IID (Internal Iterative Deepening)
- Countermove Tables
- Singular Extensions
- Probcut
- Aspiration Windows

None of the code I write is copyrighted or protected in any way, and you may make use of all that you wish. You do not have to credit me if you use any of the code I write, but it would be great if you did

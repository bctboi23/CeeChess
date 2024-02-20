# CeeChess
Hi! I am a bot written in C, heavily inspired by the Vice engine and video series done by Bluefever! If you want to try your hand at facing me, I am occasionally on lichess at https://lichess.org/@/seeChessBot! I will be moving to a fly.io app soon as well!

**Rating:**
The rating for the latest release of the engine (v1.4), scores ~150 elo better in self-play to v1.3.2, and should play at ~2300 CCRL (since self-play inflates ratings). This compares roughly to FIDE 2500, although there is no real 1-1 correspondence between these rating systems.
After the v1.4 release, I will likely either:   
1. convert the evaluation function into a custom MLP to mess around with more of the constructed datasets I have on the backend with a likely better evaluation.   
2. rewrite the engine from the ground to use bitboards (likely magic bitboard move generation), as bitboards come with a variety of perks when creating more evaluation features like mobility and more robust king safety that are computationally hard to replicate in a mailbox engine without having bitboards on hand, making the evaulation unreasonably slow.

Self play ratings for all versions, anchored at SeeChess 1.0 (1.4 is the newest version):   
(note, self-play tests were conducted at low time controls, and elo may be inflated in comparison to play against a gauntlet variety of engines. If I had to guess, CeeChess 1.4 will likely land ~2400 elo CCRL)
```
Rank      Name             Elo
 1    CeeChess-v1.4    :  ~2470
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

Gauntlet run for test ratings (1 min, 0.5sec inc), with elo centered around the v1.4 release (ratings from bayeselo):
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
- PSQT (Midgame and Endgame, from Lyudmil)
- Bishop pair heuristic (for Midgame and Endgame)
- Passed Pawn evaluation (Midgame and Endgame tables)
- Isolated pawn heuristic
- Open file heuristics (for Rook and Queen)
- King Safety (King Tropism, weighted by the number pieces left on the board + attack bonuses for semi-open files near the king)
- Tapered evaluation
- Logistic Regression Tuning (Texel method) using Simulated Annealing + Local Search, with Pseudohuber loss

**Planned Improvements (ordered by perceived feasibility):**
- Syzygy Tablebases
- SEE (Static Exchange Evaluation) (bitboards would be nice for this, but maybe feasible to do quickly in mailbox)
- Mobility (tried, too slow, would like bitboards for this)

**Other Possible Improvements (No particular order):**
- IID (Internal Iterative Deepening)
- Countermove Tables
- Singular Extensions
- Probcut
- Bitboards
- Aspiration Windows

None of the code I write is copyrighted or protected in any way, and you may make use of all that you wish. You do not have to credit me if you use any of the code I write, but it would be great if you did

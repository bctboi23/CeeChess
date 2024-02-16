# CeeChess
Hi! I am a bot written in C, heavily inspired by the Vice engine and video series done by Bluefever! If you want to try your hand at facing me, I am occasionally on lichess at https://lichess.org/@/seeChessBot! I will be moving to a fly.io app soon as well!

**Rating:**
The rating for the latest release of the engine (v1.3.2) should be around ~2330 CCRL, which compares to around 2500 FIDE classical rating.

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
- King Safety
- Mobility
- SEE (Static Exchange Evaluation)

**Other Possible Improvements (No particular order):**
- IID (Internal Iterative Deepening)
- Countermove Tables
- Singular Extensions
- Probcut
- Bitboards
- Aspiration Windows

None of the code I write is copyrighted or protected in any way, and you may make use of all that you wish. You do not have to credit me if you use any of the code I write, but it would be great if you did

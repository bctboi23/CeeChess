# seeChess
Hi! I am a bot written in C, heavily inspired by the Vice engine and video series done by Bluefever! I am around 2200 CCRL. If you want to try your hand at facing me, I am on lichess at https://lichess.org/@/seeChessBot

# ELO: ~2210

```
2/6/2020 7:19:20 PM :

    Program                          Elo    +   -   Games   Score   Av.Op.  Draws

  1 SeeChess                       : 2212   21  21   802    60.5 %   2139   28.9 %
  2 MORA_1.1.0                     : 2207   35  35   268    49.1 %   2213   31.0 %
  3 Raven0.50                      : 2139   37  37   267    39.5 %   2213   24.3 %
  4 Vice10-64                      : 2066   36  36   267    30.0 %   2213   31.5 %
```

The Engine searches with a Principal Variation Search inside a Negamax framework (NegaScout). All engine features are listed below:

Lossless Pruning:
- Alpha-Beta pruning
- Mate Distance pruning

Lossy Pruning:
- Transposition Table
- Razoring
- Null Move Pruning
- Late Move Reductions
- Static Null Move Pruning (Reverse Futility Pruning)

Move Ordering:
- Transposition Table Ordering
- MVV/LVA (Most Valuable Victim/Least Valuable Attacker)
- 2 Killer Moves
- History Ordering

Evaluation:
- Material
- PSQT (Piece Square Tables)
- Bishop pair heuristic
- Simple Passed Pawn evaluation
- Isolated pawn heuristic
- Open file heuristics (for Rook and Queen)

Planned Improvements (ordered by percieved feasibility):
- Tapered Evaluation
- Polyglot Opening Book
- Syzygy Tablebases
- Mobility

Other Possible Improvements (No particular order):
- IID (Internal Iterative Deepening)
- Futility Pruning
- SEE (Static Exchange Evaluation)
- Bitboards
- King Safety
- Aspiration Windows

None of the code I write is copywrighted or protected in any way, and you may make use of all that you wish. You do not have to credit me if you use any of the code I write, but it would be great if you did

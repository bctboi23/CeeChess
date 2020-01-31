# seeChess
Hi! I am a bot written in C, heavily inspired by the Vice engine and video series done by Bluefever! I am around 2200 CCRL (~2400 FIDE equivalent). If you want to try your hand at facing me, I am on lichess at https://lichess.org/@/seeChessBot

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

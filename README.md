DFSMaze
=======

Uses Iterative Depth First Search Algorithm with an Integer Array as a Stack and Char Array to represent Maze.
Bit manipulations are used to represent presence of walls in maze. Maze comes with varying difficulty and does not implement any parallelism techniques except for printing out the maze layout which uses OpenMP.

Benchmarks
-----------

System: Intel Core i7 @ 2.7GHz with 1600MHz RAM

Maze Sizes: 20 x 20, 100 x 100, 500 x 500 and 1000 x 1000
Generation Time: ~0.3 ms, ~2.2ms, ~35ms and ~126ms

> Upcoming: Path-Finding Algorithm for Solver


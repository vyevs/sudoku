# sudoku
simple command line sudoku solver

compile with gcc main.c or cl main.c

current solving techniques implemented:

1. lone singles
2. hidden singles
4. naked pairs
3. backtracking

initial state is provided via a file that is the 1st arg to the program - .ss format for single puzzle, will print out solution, or .sdm format for a collection of puzzles, will not print solutions, will just solve, verify and time the whole thing

verifies the final solution and reports any errors

performs 0 dynamic allocations while solving the sudoku

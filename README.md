# sudoku
simple command line sudoku solver

compile with gcc main.c or cl main.c

current solving techniques implemented:

1. lone singles
2. hidden singles
3. backtracking

initial state is provided via a file that is the 1st arg to the program

verifies the final solution and reports any errors

performs 0 dynamic allocations while solving the sudoku

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include "types.h"

struct grid {
    u32 values[9][9];
};

char *make_grid_str(const struct grid *grid) {
    assert(grid);
    char *buf = malloc(4096);
    assert(buf);
    
    char *to_print_to = buf;
    
    for (u32 row_idx = 0; row_idx < 9; row_idx++) {
        for (u32 col_idx = 0; col_idx < 9; col_idx++) {
            
            u32 val = grid->values[row_idx][col_idx];
            
            if (val == 0) {
                to_print_to[0] = ' ';
                to_print_to[1] = ' ';
                to_print_to += 2;
                
            } else {
                s64 bytes_written = sprintf(to_print_to, "%" PRIu32 " ", grid->values[row_idx][col_idx]);
                assert(bytes_written);
                
                to_print_to += bytes_written;
            }
            
            
        }
        
        if (row_idx != 8) {
            to_print_to[0] = '\n';
            to_print_to++;
        }
    }
    
    to_print_to[0] = 0;
    
    return buf;
}


struct solve_state {
    u32 values[9][9];
};

bool is_solved(const struct solve_state *solve_state) {
    assert(solve_state);
    for (u32 row_idx = 0; row_idx < 9; row_idx++) {
        for (u32 col_idx = 0; col_idx < 9; col_idx++) {
            u32 cell_value = solve_state->values[row_idx][col_idx];
            if (cell_value == 0)
                return false;
        }
    }
    return true;
}



// places the values of the box associated with cell [row_idx][col_idx] into the into arg
// into must be large enough to hold 9 values
void get_box_values(const struct solve_state *solve_state, u32 row_idx, u32 col_idx, u32 *into) {
    assert(solve_state);
    assert(row_idx >= 0);
    assert(row_idx < 9);
    assert(col_idx >= 0);
    assert(col_idx < 9);
    
    u32 box_start_row = row_idx / 3 * 3;
    u32 box_start_col = col_idx / 3 * 3;
    u32 box_end_row = box_start_row + 3;
    u32 box_end_col = box_start_col + 3;
    
    u32 box_values_idx = 0;
    for (u32 row_idx = box_start_row; row_idx < box_end_row; row_idx++) {
        for (u32 col_idx = box_start_col; col_idx < box_end_col; col_idx++) {
            u32 value = solve_state->values[row_idx][col_idx];
            into[box_values_idx] = value;
            box_values_idx++;
        }
    }
}

// places the values in the row row_idx into the into arg
// into must be large enough to hold 9 values
void get_row_values(const struct solve_state *solve_state, u32 row_idx, u32 *into) {
    assert(solve_state);
    assert(row_idx >= 0);
    assert(row_idx < 9);
    
    memcpy(into, &solve_state->values[row_idx][0], sizeof(into[0]) * 9);
}

// places the values in the col col_idx into the into arg
// into must be large enough to hold 9 values
void get_col_values(const struct solve_state *solve_state, u32 col_idx, u32 *into) {
    assert(col_idx >= 0);
    assert(col_idx < 9);
    
    u32 i = 0;
    for (u32 row_idx = 0; row_idx < 9; row_idx++) {
        into[i++] = solve_state->values[row_idx][col_idx];
    }
}


// places the possible values for the cell [row_idx, col_idx] into the in into arg
// returns the number of possibilities that were placed into into
u32 get_possible_cell_values(const struct solve_state *solve_state, u32 row_idx, u32 col_idx, u32 *into) {
    assert(solve_state);
    assert(row_idx >= 0);
    assert(row_idx < 9);
    assert(col_idx >= 0);
    assert(row_idx < 9);
    
    u32 box_values[9];
    get_box_values(solve_state, row_idx, col_idx, box_values);
    
    u32 row_values[9];
    get_row_values(solve_state, row_idx, row_values);
    
    u32 col_values[9];
    get_col_values(solve_state, col_idx, col_values);
    
    bool is_possible[9]; // index i indicates whether value i + 1 can be placed at the current cell
    memset(is_possible, true, sizeof(is_possible[0]) * sizeof(is_possible));
    
    for (u32 i = 0; i < 9; i++) {
        u32 val = box_values[i];
        if (val != 0) {
            is_possible[val-1] = false;
        }
    }
    
    for (u32 i = 0; i < 9; i++) {
        u32 val = row_values[i];
        if (val != 0)
            is_possible[val-1] = false;
    }
    
    for (u32 i = 0; i < 9; i++) {
        u32 val = col_values[i];
        if (val != 0)
            is_possible[val-1] = false;
    }
    
    u32 possibilities_idx = 0;
    for (u32 i = 0; i < 9; i++) {
        if (is_possible[i]) {
            into[possibilities_idx] = i + 1;
            possibilities_idx++;
        }
    }
    
    return possibilities_idx;
}


void move_to_next_cell(u32 *row_idx, u32 *col_idx) {
    *col_idx += 1;
    if (*col_idx > 8) {
        *col_idx = 0;
        *row_idx += 1;
    }
}


bool recursive_solve(struct solve_state *solve_state, u32 row_idx, u32 col_idx) {
    assert(solve_state);
    assert(row_idx >= 0);
    assert(col_idx >= 0);
    assert(col_idx < 9);
    
    // if we've gone off the end of the grid, that means we have solved it
    if (row_idx > 8)
        return true;
    
    u32 next_row_idx = row_idx;
    u32 next_col_idx = col_idx;
    move_to_next_cell(&next_row_idx, &next_col_idx);
    
    
    // if the current cell is already filled, that means it was filled in the initial state
    // don't do anything for it, just keep solving from the next cell
    // this can be repeated all the way to the end of the grid if the rest of the grid is solved
    if (solve_state->values[row_idx][col_idx] != 0) {
        return recursive_solve(solve_state, next_row_idx, next_col_idx);
    }
    
    u32 possible_cell_values[9];
    u32 n_possible_values = get_possible_cell_values(solve_state, row_idx, col_idx, possible_cell_values);
    
    // if we hit a cell that has no possible values for it, the path we have taken cannot succeed
    if (n_possible_values == 0)
        return false;
    
    
    for (u32 i = 0; i < n_possible_values; i++) {
        u32 possible_value = possible_cell_values[i];
        
        solve_state->values[row_idx][col_idx] = possible_value;
        bool success = recursive_solve(solve_state, next_row_idx, next_col_idx);
        
        if (success) {
            return true;
        } else {
            solve_state->values[row_idx][col_idx] = 0;
        }
    }
    
    // if we hit this, the sudoku is unsolvable
    return false;
}


struct grid *solve(const struct grid *initial_state) {
    assert(initial_state);
    
    struct solve_state solve_state;
    memcpy(solve_state.values, initial_state->values, sizeof(solve_state.values[0][0]) * 81);
    
    
    if (is_solved(&solve_state)) {
        struct grid *solved = malloc(sizeof(*solved));
        assert(solved);
        memcpy(solved->values, initial_state->values, sizeof(solved->values[0][0]) * 81);
        return solved;
    }
    
    bool success = recursive_solve(&solve_state, 0, 0);
    assert(success);
    
    
    struct grid *solved = malloc(sizeof(*solved));
    assert(solved);
    memcpy(solved->values, solve_state.values, 81 * sizeof(solved->values[0][0]));
    return solved;
}



int main(void) {
    struct grid grid;
    
    u32 easy_values[9][9] = {
        { 0, 0, 0, 8, 5, 0, 0, 0, 7 },
        { 3, 8, 2, 0, 0, 0, 0, 0, 0 },
        { 9, 0, 7, 0, 3, 0, 1, 8, 4 },
        { 0, 2, 8, 0, 0, 6, 0, 3, 0 },
        { 4, 0, 9, 0, 0, 0, 8, 0, 1 },
        { 0, 3, 0, 9, 0, 0, 4, 7, 0 },
        { 7, 1, 3, 0, 6, 0, 2, 0, 8 },
        { 0, 0, 0, 0, 0, 0, 5, 1, 6 },
        { 2, 0, 0, 0, 9, 8, 0, 0, 0 }
    };
    
    u32 medium_values[9][9] = {
        { 0, 0, 0, 0, 9, 0, 5, 0, 1 },
        { 0, 0, 0, 0, 0, 0, 0, 2, 0 },
        { 8, 3, 0, 0, 2, 0, 0, 0, 0 },
        { 0, 0, 4, 0, 1, 6, 7, 5, 0 },
        { 3, 0, 0, 0, 7, 5, 0, 1, 8 },
        { 0, 5, 0, 0, 0, 0, 0, 9, 0 },
        { 4, 1, 0, 0, 0, 0, 9, 0, 2 },
        { 7, 0, 3, 0, 0, 0, 1, 0, 0 },
        { 0, 2, 0, 6, 5, 0, 0, 0, 4 }
    };
    
    
    memcpy(grid.values, medium_values, 81 * sizeof(grid.values[0][0]));
    
    char *grid_str = make_grid_str(&grid);
    printf("%s\n\n\n", grid_str);
    
    clock_t start = clock();
    struct grid *solved = solve(&grid);
    clock_t end = clock();
    
    float duration = (float) (end - start) / CLOCKS_PER_SEC;
    printf("%f\n", duration);
    
    
    grid_str = make_grid_str(solved);
    
    printf("%s\n", grid_str);
    
    return EXIT_SUCCESS;
}
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
    
    // box_values are the values in each box from top left to bottom right, reading order
    // e.g. if box 4 (the center box) looks like { 1, 0, 3 }
    //                                           { 4, 0, 2 }
    //                                           { 0, 9, 0 }
    // then box_values[4] = { true(1), true(2), true(3), true(4), false(5), false(6), false(7), false(8), true(9) }
    bool box_values[9][9];
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

// populates the into arg with 9 bools about whether a particular value can be placed at [row_idx][col_idx]
// e.g.: if into[3] is true, it is possible to place value 4 at [row_idx][col_idx]
void get_possible_cell_values(const struct solve_state *solve_state, u32 row_idx, u32 col_idx, bool *into) {
    assert(solve_state);
    assert(row_idx >= 0);
    assert(row_idx < 9);
    assert(col_idx >= 0);
    assert(row_idx < 9);
    
    memset(into, true, 9 * sizeof(into[0]));
    
    u32 box_idx = (row_idx / 3) * 3 + col_idx / 3;
    const bool *box_values = solve_state->box_values[box_idx];
    for (u32 i = 0; i < 9; i++) {
        bool have_value_in_box = box_values[i];
        if (have_value_in_box)
            into[i] = false;
    }
    
    // look at the row that this cell is in
    for (u32 col_idx = 0; col_idx < 9; col_idx++) {
        u32 val = solve_state->values[row_idx][col_idx];
        if (val != 0)
            into[val-1] = false;
    }
    
    // look at the column that this cell is in
    for (u32 row_idx = 0; row_idx < 9; row_idx++) {
        u32 val = solve_state->values[row_idx][col_idx];
        if (val != 0)
            into[val-1] = false;
    }
}


void move_to_next_cell(u32 *row_idx, u32 *col_idx) {
    *col_idx += 1;
    if (*col_idx > 8) {
        *col_idx = 0;
        *row_idx += 1;
    }
}

// set value sets the solve_state grid's value at [row_idx][col_idx] to value
// this function is used because there is bookeeping to do when setting a value
void set_value(struct solve_state *solve_state, u32 row_idx, u32 col_idx, u32 value) {
    assert(solve_state);
    assert(row_idx >= 0);
    assert(row_idx < 9);
    assert(col_idx >= 0);
    assert(col_idx < 9);
    assert(value >= 1);
    assert(value <= 9);
    assert(solve_state->values[row_idx][col_idx] == 0);
    
    solve_state->values[row_idx][col_idx] = value;
    
    u32 box_idx = (row_idx / 3) * 3 + col_idx / 3;
    solve_state->box_values[box_idx][value-1] = true;
}

// unsets the value at [row_idx][col_idx] in the solve_state grid
// this is used because there is bookeeping to do when a value is unset
void unset_value(struct solve_state *solve_state, u32 row_idx, u32 col_idx) {
    assert(solve_state);
    assert(row_idx >= 0);
    assert(row_idx < 9);
    assert(col_idx >= 0);
    assert(col_idx < 9);
    assert(solve_state->values[row_idx][col_idx] != 0);
    
    
    u32 value_at_cell = solve_state->values[row_idx][col_idx];
    solve_state->values[row_idx][col_idx] = 0;
    
    u32 box_idx = (row_idx / 3) * 3 + col_idx / 3;
    solve_state->box_values[box_idx][value_at_cell-1] = false;
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
    
    bool possible_values[9];
    get_possible_cell_values(solve_state, row_idx, col_idx, possible_values);
    
    for (u32 i = 0; i < 9; i++) {
        bool is_possible = possible_values[i];
        if (!is_possible)
            continue;
        
        u32 value = i + 1;
        
        set_value(solve_state, row_idx, col_idx, value);
        bool success = recursive_solve(solve_state, next_row_idx, next_col_idx);
        
        if (success) {
            return true;
        } else {
            unset_value(solve_state, row_idx, col_idx);
        }
    }
    
    // if we hit this, the path we have taken is unsolvable
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
    
    memset(solve_state.box_values, false, 81 * sizeof(solve_state.box_values[0][0]));
    
    // populate initial box values
    for (u32 box_row = 0; box_row < 3; box_row++) {
        for (u32 box_col = 0; box_col < 3; box_col++) {
            
            u32 box_idx = box_row * 3 + box_col;
            
            u32 values_row_start = box_row * 3;
            u32 values_row_end = values_row_start + 3;
            u32 values_col_start = box_col * 3;
            u32 values_col_end = values_col_start + 3;
            
            for (u32 values_row = values_row_start; values_row < values_row_end; values_row++) {
                for (u32 values_col = values_col_start; values_col < values_col_end; values_col++) {
                    u32 value = solve_state.values[values_row][values_col];
                    if (value != 0)
                        solve_state.box_values[box_idx][value-1] = true;
                }
            }
            
        }
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
    
    
    memcpy(grid.values, easy_values, 81 * sizeof(grid.values[0][0]));
    
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
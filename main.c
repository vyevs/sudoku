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


typedef enum { ROW_ERROR, COL_ERROR, BOX_ERROR, NOT_FILLED_ERROR } error_type;

struct is_solved_result {
    bool is_solved;
    
    // filled if is_solved == false
    error_type err_type;
    u32 err_value;
    u32 err_idx;
};

struct is_solved_result is_solved(const struct grid *grid) {
    assert(grid);
    
    bool have_value[9];
    
    struct is_solved_result res;
    res.is_solved = false;
    
    for (u32 row_idx = 0; row_idx < 9; row_idx++) {
        memset(have_value, false, 9 * sizeof(have_value[0]));
        
        for (u32 col_idx = 0; col_idx < 9; col_idx++) {
            u32 value_at_cell = grid->values[row_idx][col_idx];
            if(value_at_cell == 0) {
                res.err_type = NOT_FILLED_ERROR; // grid is not filled out fully
                return res;
            }
            
            if (have_value[value_at_cell-1]) {
                res.err_type = ROW_ERROR;
                res.err_value = value_at_cell;
                res.err_idx = row_idx;
                return res;
            } else {
                have_value[value_at_cell-1] = true;
            }
        }
    }
    
    for (u32 col_idx = 0; col_idx < 9; col_idx++) {
        memset(have_value, false, 9 * sizeof(have_value[0]));
        
        for (u32 row_idx = 0; row_idx < 9; row_idx++) {
            u32 value_at_cell = grid->values[row_idx][col_idx];
            if(value_at_cell == 0) {
                res.err_type = NOT_FILLED_ERROR; // grid is not filled out fully
                return res;
            }
            
            if(have_value[value_at_cell-1]) {
                res.err_type = COL_ERROR;
                res.err_value = value_at_cell;
                res.err_idx = col_idx;
                return res;
            }else {
                have_value[value_at_cell-1] = true;
            }
        }
    }
    
    u32 box_row_start = 0;
    u32 box_col_start = 0;
    u32 box_row_end = 3;
    u32 box_col_end = 3;
    for (u32 box_idx = 0; box_idx < 9; box_idx++) {
        memset(have_value, false, 9 * sizeof(have_value[0]));
        
        for (u32 row_idx = box_row_start; row_idx < box_row_end; row_idx++) {
            for (u32 col_idx = box_col_start; col_idx < box_col_end; col_idx++) {
                u32 value_at_cell = grid->values[row_idx][col_idx];
                if(value_at_cell == 0) {
                    res.err_type = NOT_FILLED_ERROR; // grid is not filled out fully
                    return res;
                }
                
                if (have_value[value_at_cell-1]) {
                    res.err_type = BOX_ERROR;
                    res.err_value = value_at_cell;
                    res.err_idx = box_idx;
                    return res;
                } else {
                    have_value[value_at_cell-1] = true;
                }
            }
        }
        
        box_col_start += 3;
        if (box_col_start == 9) {
            box_col_start = 0;
            box_col_end = 3;
            box_row_start += 3;
            box_row_end += 3;
        }
        
    }
    
    res.is_solved = true;
    return res;
}

struct solve_state {
    u32 values[9][9];
    
    // colissions[row][col][i] is how many colissions there would be if we put value i + 1 at [row][col]
    // if collisions[row][col][i] == 0 then you can place value i + 1 at [row][col] without issue
    s32 collisions[9][9][9];
};



bool is_filled_out(const struct solve_state *solve_state) {
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

u32 box_row_lookup[9][9] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3},
    {6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6}
};

u32 box_col_lookup[9][9] = {
    {0, 0, 0, 3, 3, 3, 6, 6, 6},
    {0, 0, 0, 3, 3, 3, 6, 6, 6},
    {0, 0, 0, 3, 3, 3, 6, 6, 6},
    {0, 0, 0, 3, 3, 3, 6, 6, 6},
    {0, 0, 0, 3, 3, 3, 6, 6, 6},
    {0, 0, 0, 3, 3, 3, 6, 6, 6},
    {0, 0, 0, 3, 3, 3, 6, 6, 6},
    {0, 0, 0, 3, 3, 3, 6, 6, 6},
    {0, 0, 0, 3, 3, 3, 6, 6, 6}
};

// modifies solve_state->collisions for cells related to [row_idx][col_idx]
// related cells are the ones in teh same row, column and box as [row_idx][col_idx]
// value is the value that has been placed OR removed from cell [row_idx][col_idx]
// if value is 0, that means the cell value has been erased/unset
// set is whether the value has been set or unset/erased
void modify_related_cells_collisions(struct solve_state *solve_state, u32 row_idx, u32 col_idx, u32 value, bool set) {
    assert(solve_state);
    assert(row_idx >= 0);
    assert(row_idx < 9);
    assert(col_idx >= 0);
    assert(col_idx < 9);
    
    // if the value is being set, we want to add 1 to the collisions of related cells for that value
    // otherwise subtract 1 because that's 1 less collision
    s32 delta;
    if (set)
        delta = 1;
    else
        delta = -1;
    
    // we need to look at every cell that is not filled that is in our row, col & box
    // BUT we only need to look forward in the row, column
    // this could be possible for the box, but not done right now
    
    // look at forward values in our row
    for (u32 other_col = 0; other_col < 9; other_col++) {
        solve_state->collisions[row_idx][other_col][value-1] += delta;
    }
    
    // look at forward values in our col
    for (u32 other_row = 0; other_row < 9; other_row++) {
        solve_state->collisions[other_row][col_idx][value-1] += delta;
    }
    
    
    // look at values in our box, it could be possible that we look through
    // only the forward values, but not doing that yet @TODO
    u32 box_row_start = box_row_lookup[row_idx][col_idx];
    u32 box_row_end = box_row_start + 3;
    u32 box_col_start = box_col_lookup[row_idx][col_idx];
    u32 box_col_end = box_col_start + 3;
    for (u32 box_row = box_row_start; box_row < box_row_end; box_row++) {
        for (u32 box_col = box_col_start; box_col < box_col_end; box_col++) {
            solve_state->collisions[box_row][box_col][value-1] += delta;
        }
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
    
    modify_related_cells_collisions(solve_state, row_idx, col_idx, value, true);
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
    
    modify_related_cells_collisions(solve_state, row_idx, col_idx, value_at_cell, false);
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
    u32 next_col_idx = col_idx + 1;
    if (next_col_idx > 8) {
        next_col_idx = 0;
        next_row_idx += 1;
    }
    
    // if the current cell is already filled, that means it was filled in the initial state
    // don't do anything for it, just keep solving from the next cell
    // this can be repeated all the way to the end of the grid if the rest of the grid is solved
    if (solve_state->values[row_idx][col_idx] != 0) {
        return recursive_solve(solve_state, next_row_idx, next_col_idx);
    }
    
    
    s32 *collisions = solve_state->collisions[row_idx][col_idx];
    
    for (u32 i = 0; i < 9; i++) {
        if (collisions[i] > 0)
            // if we tried to place i + 1 at [row_idx][col_idx], there would be > 0 collisions, so skip it
            continue;
        
        u32 value = i + 1;
        
        set_value(solve_state, row_idx, col_idx, value);
        //exit(1);
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

// fills out all the cells that can only have one possible value
// does this repeatedly so that if filling out one cell allows a second to be filled
// the second cell is filled as well, and so on
void reveal_naked_singles(struct solve_state *solve_state) {
    bool revealed_at_least_one;
    do {
        revealed_at_least_one = false;
        
        for (u32 row_idx = 0; row_idx < 9; row_idx++) {
            for (u32 col_idx = 0; col_idx < 9; col_idx++) {
                
                if (solve_state->values[row_idx][col_idx] != 0)
                    continue;
                
                s32 *collisions = solve_state->collisions[row_idx][col_idx];
                
                u32 n_possible = 0;
                for (u32 i = 0; i < 9; i++) {
                    if(collisions[i] == 0)
                        n_possible++;
                }
                
                if (n_possible != 1)
                    continue;
                
                revealed_at_least_one = true;
                
                u32 single_val_idx = -1;
                for (u32 i = 0; i < 9; i++) {
                    if (collisions[i] == 0) {
                        single_val_idx = i;
                        break;
                    }
                }
                assert(single_val_idx != -1);
                
                u32 value = single_val_idx + 1;
                
                set_value(solve_state, row_idx, col_idx, value);
            }
        }
        
    } while (revealed_at_least_one);
}

struct grid *solve(const struct grid *initial_state) {
    assert(initial_state);
    
    struct solve_state solve_state;
    memcpy(solve_state.values, initial_state->values, sizeof(solve_state.values[0][0]) * 81);
    
    
    if (is_filled_out(&solve_state))
        goto done;
    
    // our initial state is that all the values are possible for this cell
    memset(solve_state.collisions, 0, 9 * 9 * 9 * sizeof(solve_state.collisions[0][0][0]));
    
    // initialize collisions for each cell
    for (u32 row_idx = 0; row_idx < 9; row_idx++) {
        for (u32 col_idx = 0; col_idx < 9; col_idx++) {
            
            // if this cell already has a value, that means it is part of the initial state
            // so just skip it
            if (solve_state.values[row_idx][col_idx] != 0)
                continue;
            
            
            s32 *collisions = solve_state.collisions[row_idx][col_idx];
            
            // look through row, col & box of this cell to determine which values CANNOT be present in the cell
            
            // look through row which we are in
            for (u32 other_col = 0; other_col < 9; other_col++) {
                u32 cell_value = solve_state.values[row_idx][other_col];
                if (cell_value != 0) {
                    collisions[cell_value-1]++;
                }
            }
            
            // look through col which we are in
            for (u32 other_row = 0; other_row < 9; other_row++) {
                u32 cell_value = solve_state.values[other_row][col_idx];
                if (cell_value != 0) {
                    collisions[cell_value-1]++;
                }
            }
            
            
            // look through the box which are in
            u32 box_row_start = row_idx / 3 * 3;
            u32 box_row_end = box_row_start + 3;
            u32 box_col_start = col_idx / 3 * 3;
            u32 box_col_end = box_col_start + 3;
            for (u32 box_row = box_row_start; box_row < box_row_end; box_row++) {
                for (u32 box_col = box_col_start; box_col < box_col_end; box_col++) {
                    u32 cell_value = solve_state.values[box_row][box_col];
                    if (cell_value != 0)
                        collisions[cell_value-1]++;
                }
            }
        }
    }
    
    
    reveal_naked_singles(&solve_state);
    if (is_filled_out(&solve_state)) {
        goto done;
    }
    
    bool success = recursive_solve(&solve_state, 0, 0);
    assert(success);
    
    done:
    struct grid *solved = malloc(sizeof(*solved));
    assert(solved);
    memcpy(solved->values, solve_state.values, 81 * sizeof(solved->values[0][0]));
    return solved;
}



int main(void) {
    
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
    
    u32 hard_values[9][9] = {
        { 9, 0, 4, 0, 0, 0, 7, 0, 2 },
        { 6, 0, 0, 0, 0, 0, 9, 5, 0 },
        { 0, 3, 0, 0, 2, 0, 0, 0, 4 },
        { 0 ,0, 6, 9, 0, 3, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0 ,0, 0, 8, 0, 5, 1, 0, 0 },
        { 7, 0, 0, 0, 9, 0, 0, 8, 0 },
        { 0, 4, 5, 0, 0, 0, 0, 0, 9 },
        { 3, 0, 1, 0, 0, 0, 5, 0, 6 }
    };
    
    struct grid grid;
    memcpy(grid.values, medium_values, 81 * sizeof(grid.values[0][0]));
    
    char *grid_str = make_grid_str(&grid);
    printf("%s\n\n", grid_str);
    
    struct grid *solution;
    
    
    clock_t start = clock();
    for (u32 i = 0; i < 1; i++) {
        solution = solve(&grid);
    }
    clock_t end = clock();
    
    float duration = (float) (end - start) / CLOCKS_PER_SEC;
    printf("%f\n", duration);
    
    grid_str = make_grid_str(solution);
    printf("%s\n", grid_str);
    
    {
        struct is_solved_result solved = is_solved(solution);
        if (!solved.is_solved) {
            printf("INCORRECT SOLUTION!!!\n");
            
            switch (solved.err_type) {
                case ROW_ERROR: {
                    printf("row %"PRIu32" has value %"PRIu32" multiple times\n", solved.err_idx, solved.err_value);
                }
                break;
                
                case COL_ERROR: {
                    printf("col %"PRIu32" has value %"PRIu32" multiple times\n", solved.err_idx, solved.err_value);
                }
                break;
                
                case BOX_ERROR: {
                    printf("box %"PRIu32" has value %"PRIu32" multiple times\n", solved.err_idx, solved.err_value);
                }
                break;
            }
        } else {
            printf("LOOKS GOOD TO ME!\n");
        }
    }
    
    return EXIT_SUCCESS;
}
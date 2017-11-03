#ifndef CELL_BLOCK_COORDINATE_SYSTEM_H_DEF
#define CELL_BLOCK_COORDINATE_SYSTEM_H_DEF


#include "types.h"
#include "vectors.h"


void
small_global_cell_coord_to_cell_block_coords(s32vec2 small_global_cell_coord, s32vec2 *resulting_cell_block_coordinate, s32vec2 *resulting_cell_coordinate);


b32
cell_position_greater_than_or_equal_to(s32vec2 cell_block_position_test, s32vec2 cell_position_test,
                                       s32vec2 cell_block_position_bound, s32vec2 cell_position_bound);


b32
cell_position_less_than(s32vec2 cell_block_position_test, s32vec2 cell_position_test,
                        s32vec2 cell_block_position_bound, s32vec2 cell_position_bound);


#endif
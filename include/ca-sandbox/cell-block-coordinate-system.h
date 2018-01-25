#ifndef CELL_BLOCK_COORDINATE_SYSTEM_H_DEF
#define CELL_BLOCK_COORDINATE_SYSTEM_H_DEF

#include "engine/types.h"
#include "engine/vectors.h"

#include "ca-sandbox/universe.h"


struct UniversePosition
{
  s32vec2 cell_block_position;
  vec2 cell_position;
};


void
normalise_cell_coord(Universe *universe, s32 *cell_block_coord, s32 *cell_coord);


void
normalise_cell_coord(Universe *universe, s32vec2 *cell_block_coord, s32vec2 *cell_coord);


void
small_global_cell_coord_to_cell_block_coords(Universe *universe, s32vec2 small_global_cell_coord, s32vec2 *resulting_cell_block_coordinate, s32vec2 *resulting_cell_coordinate);


b32
cell_position_greater_than_or_equal_to(s32 cell_block_position_test, s32 cell_position_test,
                                       s32 cell_block_position_bound, s32 cell_position_bound);


b32
cell_position_greater_than_or_equal_to(s32vec2 cell_block_position_test, s32vec2 cell_position_test,
                                       s32vec2 cell_block_position_bound, s32vec2 cell_position_bound);


b32
cell_position_less_than(s32 cell_block_position_test, s32 cell_position_test,
                        s32 cell_block_position_bound, s32 cell_position_bound);


b32
cell_position_less_than(s32vec2 cell_block_position_test, s32vec2 cell_position_test,
                        s32vec2 cell_block_position_bound, s32vec2 cell_position_bound);


b32
cell_position_equal_to(UniversePosition a, UniversePosition b);


s32vec2
cell_block_round(vec2 real_cell_block);


#endif
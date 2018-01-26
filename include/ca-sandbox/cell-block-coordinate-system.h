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


inline UniversePosition
vec2_add(UniversePosition a, UniversePosition b)
{
  UniversePosition result = {
    .cell_block_position = vec2_add(a.cell_block_position, b.cell_block_position),
    .cell_position = vec2_add(a.cell_position, b.cell_position)
  };
  return result;
}

inline UniversePosition
vec2_multiply(UniversePosition a, vec2 b)
{
  UniversePosition result = {
    .cell_block_position = vec2_multiply(a.cell_block_position, vec2_to_s32vec2(b)),
    .cell_position = vec2_multiply(a.cell_position, b)
  };
  return result;
}


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


void
correct_cell_block_square_order(s32vec2& start_block, s32vec2& start_cell, s32vec2& end_block, s32vec2& end_cell);


void
correct_cell_block_square_order(UniversePosition& start, UniversePosition& end);


void
quantise_0to1_cell_position(vec2& continuous, u32 cell_block_dim);


#endif
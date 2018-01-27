#include "ca-sandbox/cell-block-coordinate-system.h"

#include "ca-sandbox/cell-blocks.h"

/// @file
/// @brief  Functions for managing CellBlock and Cell coordinates
///


/// Normalise a (cell block, cell position) so that the cell position is less than cell_block_dim
void
normalise_cell_coord(Universe *universe, s32 *cell_block_coord, s32 *cell_coord)
{
  // Account for zero cross over
  s32 zero_offset_cell_coord;
  if (*cell_coord < 0)
  {
    zero_offset_cell_coord = *cell_coord - universe->cell_block_dim;
  }
  else
  {
    zero_offset_cell_coord = *cell_coord;
  }

  s32 delta_blocks = zero_offset_cell_coord / (s32)universe->cell_block_dim;

  (*cell_block_coord) += delta_blocks;
  (*cell_coord) -= delta_blocks * universe->cell_block_dim;
}


void
normalise_cell_coord(Universe *universe, s32vec2 *cell_block_coord, s32vec2 *cell_coord)
{
  normalise_cell_coord(universe, &cell_block_coord->x, &cell_coord->x);
  normalise_cell_coord(universe, &cell_block_coord->y, &cell_coord->y);
}


/// @brief Converts a global cell coordinate into a (cell block + cell) coordinate
///
/// NOTE: s32vec2 is not able to index the entire region which the Universe is able to map, hence:
///         'small'.
///
/// @param[in] small_global_cell_coord  Global cell coordinate to convert.
/// @param[out] resulting_cell_block_coordinate  Pointer to be filled out with the CellBlock
///                                                coordinate containing the cell coordinate being
///                                                converted.
/// @param[out] resulting_cell_coordinate  Pointer to be filled out with the coordinate within the
///                                          resulting_cell_block_coordinate of the cell coordinate
///                                          being converted.
///
void
small_global_cell_coord_to_cell_block_coords(Universe *universe, s32vec2 small_global_cell_coord, s32vec2 *resulting_cell_block_coordinate, s32vec2 *resulting_cell_coordinate)
{
  // Account for zero cross over
  s32vec2 zero_offset_small_global_cell_coord = small_global_cell_coord;
  if (zero_offset_small_global_cell_coord.x < 0)
  {
    zero_offset_small_global_cell_coord.x -= universe->cell_block_dim;
  }
  if (zero_offset_small_global_cell_coord.y < 0)
  {
    zero_offset_small_global_cell_coord.y -= universe->cell_block_dim;
  }

  *resulting_cell_block_coordinate = vec2_divide(zero_offset_small_global_cell_coord, (s32)universe->cell_block_dim);
  *resulting_cell_coordinate = vec2_subtract(small_global_cell_coord,
                                             vec2_multiply(*resulting_cell_block_coordinate, (s32)universe->cell_block_dim));
}


/// @brief Greater-than-or-equal-to (>=) test for 1-D of a global (cell block + cell) coordinate
///
/// @returns true if 'test' position is greater than or equal to 'bound' position
///
/// @param[in] cell_block_position_test  Cell block position to test
/// @param[in] cell_position_test  Cell position within cell_block_position_test
/// @param[in] cell_block_position_bound  Cell block position to test against
/// @param[in] cell_position_bound  Cell position within cell_block_position_bound
///
b32
cell_position_greater_than_or_equal_to(s32 cell_block_position_test, s32 cell_position_test,
                                       s32 cell_block_position_bound, s32 cell_position_bound)
{
  b32 result = (cell_block_position_test > cell_block_position_bound ||
                (cell_block_position_test == cell_block_position_bound &&
                 cell_position_test >= cell_position_bound));
  return result;
}

b32
cell_position_greater_than_or_equal_to(s32 cell_block_position_test, r32 cell_position_test,
                                       s32 cell_block_position_bound, r32 cell_position_bound)
{
  b32 result = (cell_block_position_test > cell_block_position_bound ||
                (cell_block_position_test == cell_block_position_bound &&
                 cell_position_test >= cell_position_bound));
  return result;
}


/// @brief Greater-than-or-equal-to (>=) test for global (cell block + cell) coordinates
///
/// @returns true if 'test' position is greater than or equal to 'bound' position
///
/// @param[in] cell_block_position_test  Cell block coordinate to test
/// @param[in] cell_position_test  Cell position within cell_block_position_test
/// @param[in] cell_block_position_bound  Cell block coordinate to test against
/// @param[in] cell_position_bound  Cell position within cell_block_position_bound
///
b32
cell_position_greater_than_or_equal_to(s32vec2 cell_block_position_test, s32vec2 cell_position_test,
                                       s32vec2 cell_block_position_bound, s32vec2 cell_position_bound)
{
  b32 result = (cell_position_greater_than_or_equal_to(cell_block_position_test.x, cell_position_test.x,
                                                       cell_block_position_bound.x, cell_position_bound.x) &&
                cell_position_greater_than_or_equal_to(cell_block_position_test.y, cell_position_test.y,
                                                       cell_block_position_bound.y, cell_position_bound.y));
  return result;
}

b32
cell_position_greater_than_or_equal_to(s32vec2 cell_block_position_test, vec2 cell_position_test,
                                       s32vec2 cell_block_position_bound, vec2 cell_position_bound)
{
  b32 result = (cell_position_greater_than_or_equal_to(cell_block_position_test.x, cell_position_test.x,
                                                       cell_block_position_bound.x, cell_position_bound.x) &&
                cell_position_greater_than_or_equal_to(cell_block_position_test.y, cell_position_test.y,
                                                       cell_block_position_bound.y, cell_position_bound.y));
  return result;
}


/// @brief Less-than (<) test for 1-D of a global (cell block + cell) coordinate
///
/// @returns true if 'test' position is less than 'bound' position
///
/// @param[in] cell_block_position_test  Cell block position to test
/// @param[in] cell_position_test  Cell position within cell_block_position_test
/// @param[in] cell_block_position_bound  Cell block position to test against
/// @param[in] cell_position_bound  Cell position within cell_block_position_bound
///
b32
cell_position_less_than(s32 cell_block_position_test, s32 cell_position_test,
                        s32 cell_block_position_bound, s32 cell_position_bound)
{
  b32 result = (cell_block_position_test < cell_block_position_bound ||
                (cell_block_position_test == cell_block_position_bound &&
                 cell_position_test < cell_position_bound));
  return result;
}


/// @brief Less-than (<) test for global (cell block + cell) coordinates
///
/// @returns true if 'test' position is less than 'bound' position
///
/// @param[in] cell_block_position_test  Cell block coordinate to test
/// @param[in] cell_position_test  Cell position within cell_block_position_test
/// @param[in] cell_block_position_bound  Cell block coordinate to test against
/// @param[in] cell_position_bound  Cell position within cell_block_position_bound
///
b32
cell_position_less_than(s32vec2 cell_block_position_test, s32vec2 cell_position_test,
                        s32vec2 cell_block_position_bound, s32vec2 cell_position_bound)
{
  b32 result = (cell_position_less_than(cell_block_position_test.x, cell_position_test.x,
                                        cell_block_position_bound.x, cell_position_bound.x) &&
                cell_position_less_than(cell_block_position_test.y, cell_position_test.y,
                                        cell_block_position_bound.y, cell_position_bound.y));
  return result;
}


b32
cell_position_equal_to(UniversePosition a, UniversePosition b)
{
  b32 result = (a.cell_block_position.x == b.cell_block_position.x &&
                a.cell_block_position.y == b.cell_block_position.y &&
                a.cell_position.x == b.cell_position.x &&
                a.cell_position.y == b.cell_position.y);
  return result;
}


s32
cell_block_round(r32 real_cell_block)
{
  if (real_cell_block < 0)
  {
    real_cell_block -= 1;
  }

  s32 result = (s32)(real_cell_block);
  return result;
}


/// Round a real number of cell blocks handling zero cross over correctly.
///   i.e: f(1.5) -> 1
///        f(0.5) -> 0
///        f(-0.5) -> -1
///        f(-1.5) -> -2
///
s32vec2
cell_block_round(vec2 real_cell_block)
{
  s32vec2 result;
  result.x = cell_block_round(real_cell_block.x);
  result.y = cell_block_round(real_cell_block.y);
  return result;
}


void
correct_cell_block_square_order(s32& start_block, s32& start_cell, s32& end_block, s32& end_cell)
{
  if (cell_position_greater_than_or_equal_to(start_block, start_cell, end_block, end_cell))
  {
    s32 temp_end_block = end_block;
    s32 temp_end_cell = end_cell;

    end_block = start_block;
    end_cell = start_cell;

    start_block = temp_end_block;
    start_cell = temp_end_cell;
  }
}

void
correct_cell_block_square_order(s32& start_block, r32& start_cell, s32& end_block, r32& end_cell)
{
  if (cell_position_greater_than_or_equal_to(start_block, start_cell, end_block, end_cell))
  {
    s32 temp_end_block = end_block;
    r32 temp_end_cell = end_cell;

    end_block = start_block;
    end_cell = start_cell;

    start_block = temp_end_block;
    start_cell = temp_end_cell;
  }
}


/// Updates the cell block coordinates so that start is always closest to the origin
void
correct_cell_block_square_order(s32vec2& start_block, s32vec2& start_cell, s32vec2& end_block, s32vec2& end_cell)
{
  correct_cell_block_square_order(start_block.x, start_cell.x, end_block.x, end_cell.x);
  correct_cell_block_square_order(start_block.y, start_cell.y, end_block.y, end_cell.y);
}


void
correct_cell_block_square_order(UniversePosition& start, UniversePosition& end)
{
  correct_cell_block_square_order(start.cell_block_position.x, start.cell_position.x, end.cell_block_position.x, end.cell_position.x);
  correct_cell_block_square_order(start.cell_block_position.y, start.cell_position.y, end.cell_block_position.y, end.cell_position.y);
}


void
quantise_0to1_cell_position(r32& continuous, u32 cell_block_dim)
{
  continuous = (r32)(s32)(continuous * cell_block_dim) / cell_block_dim;
}


void
quantise_0to1_cell_position(vec2& continuous, u32 cell_block_dim)
{
  quantise_0to1_cell_position(continuous.x, cell_block_dim);
  quantise_0to1_cell_position(continuous.y, cell_block_dim);
}


void
get_cell_blocks_dimentions(CellBlocks *cell_blocks, s32vec2 *lowest_coords, s32vec2 *highest_coords)
{
  *lowest_coords = {};
  *highest_coords = {};

  b32 first_block_found = false;
  for (u32 cell_block_slot = 0;
       cell_block_slot < cell_blocks->hashmap_size;
       ++cell_block_slot)
  {
    CellBlock *cell_block = cell_blocks->hashmap[cell_block_slot];

    while (cell_block != 0)
    {
      if (!first_block_found)
      {
        first_block_found = true;
        *lowest_coords = cell_block->block_position;
        *highest_coords = cell_block->block_position;
      }
      else
      {
        if (cell_block->block_position.x < lowest_coords->x)
        {
          lowest_coords->x = cell_block->block_position.x;
        }
        else if (cell_block->block_position.x > highest_coords->x)
        {
          highest_coords->x = cell_block->block_position.x;
        }

        if (cell_block->block_position.y < lowest_coords->y)
        {
          lowest_coords->y = cell_block->block_position.y;
        }
        else if (cell_block->block_position.y > highest_coords->y)
        {
          highest_coords->y = cell_block->block_position.y;
        }
      }

      cell_block = cell_block->next_block;
    }
  }
}


void
midpoint(s32 start_block, s32 start_cell, s32 end_block, s32 end_cell, r32& midpoint_block, r32& midpoint_cell)
{
  midpoint_block = (end_block - start_block) * 0.5;
  midpoint_cell = (end_cell - start_cell) * 0.5;
}


void
midpoint(s32vec2 start_block, s32vec2 start_cell, s32vec2 end_block, s32vec2 end_cell, vec2& midpoint_block, vec2& midpoint_cell)
{
  midpoint(start_block.x, start_cell.x, end_block.x, end_cell.x, midpoint_block.x, midpoint_cell.x);
  midpoint(start_block.y, start_cell.y, end_block.y, end_cell.y, midpoint_block.y, midpoint_cell.y);
}
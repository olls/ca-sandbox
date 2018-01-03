#include "cell-block-coordinate-system.h"

#include "universe.h"


/// @file
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
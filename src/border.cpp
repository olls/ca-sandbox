#include "border.h"

#include "types.h"
#include "vectors.h"
#include "print.h"
#include "assert.h"

#include "simulate.h"
#include "rule.h"


b32
within_border(Border border, s32vec2 cell_block_position, s32vec2 cell_position)
{
  b32 result = (cell_position_greater_than_or_equal_to(cell_block_position, cell_position,
                                                       border.min_corner_block, border.min_corner_cell) &&
                cell_position_less_than(cell_block_position, cell_position,
                                        border.max_corner_block, border.max_corner_cell));

  return result;
}


void
wrap_cell_position_around_torus(Border *border, Universe *universe, s32vec2 *cell_block_position, s32vec2 *cell_position)
{
  s32 wrapped_cell_block_x = cell_block_position->x;
  s32 wrapped_cell_block_y = cell_block_position->y;
  s32 wrapped_cell_x = cell_position->x;
  s32 wrapped_cell_y = cell_position->y;

  // Size of the simulated area
  s32vec2 delta_block = vec2_subtract(border->max_corner_block, border->min_corner_block);
  s32vec2 delta_cell = vec2_subtract(border->max_corner_cell, border->min_corner_cell);

  // Wrap down to minimum bound

  if (cell_position_greater_than_or_equal_to(cell_block_position->x, cell_position->x,
                                             border->max_corner_block.x, border->max_corner_cell.x))
  {
    wrapped_cell_block_x = cell_block_position->x - delta_block.x;
    wrapped_cell_x = cell_position->x - delta_cell.x;

    normalise_cell_coord(universe, &wrapped_cell_block_x, &wrapped_cell_x);
  }

  if (cell_position_greater_than_or_equal_to(cell_block_position->y, cell_position->y,
                                             border->max_corner_block.y, border->max_corner_cell.y))
  {
    wrapped_cell_block_y = cell_block_position->y - delta_block.y;
    wrapped_cell_y = cell_position->y - delta_cell.y;

    normalise_cell_coord(universe, &wrapped_cell_block_y, &wrapped_cell_y);
  }

  // Wrap up to maximum bound

  if (cell_position_less_than(cell_block_position->x, cell_position->x,
                              border->min_corner_block.x, border->min_corner_cell.x))
  {
    wrapped_cell_block_x = cell_block_position->x + delta_block.x;
    wrapped_cell_x = cell_position->x + delta_cell.x;

    normalise_cell_coord(universe, &wrapped_cell_block_x, &wrapped_cell_x);
  }

  if (cell_position_less_than(cell_block_position->y, cell_position->y,
                              border->min_corner_block.y, border->min_corner_cell.y))
  {
    wrapped_cell_block_y = cell_block_position->y + delta_block.y;
    wrapped_cell_y = cell_position->y + delta_cell.y;

    normalise_cell_coord(universe, &wrapped_cell_block_y, &wrapped_cell_y);
  }

  *cell_block_position = (s32vec2){wrapped_cell_block_x, wrapped_cell_block_y};
  *cell_position = (s32vec2){wrapped_cell_x, wrapped_cell_y};
}


/// Get a neighbouring cell's state using a cell delta from the subject cell
///
/// This function handles the border conditions:
/// - Wrap around the torus border
/// - Returns false if the neighbouring cell is outside of the border, indicating the subject cell
///     should not be simulated.
///
/// @param[in] border
/// @param[in] universe
/// @param[in] neighbouring_cell_delta  A delta in cells from the subject_block
/// @param[in] subject_block  The position of the block containing the subject_cell
/// @param[in] subject_cell  The cell to base the delta off of
/// @param[out] resulting_state  The state of the neighbouring cell
///
/// @returns Boolean indicating whether the subject cell should be simulated.
bool
get_neighbouring_cell_state(Border *border, Universe *universe, s32vec2 neighbouring_cell_delta, s32vec2 subject_block, s32vec2 subject_cell, CellState *resulting_state)
{
  b32 result = true;

  // Calculate the absolute position of the neighbouring cell
  s32vec2 cell_coord = vec2_add(subject_cell, neighbouring_cell_delta);
  s32vec2 cell_block_position = subject_block;

  // Normalise position to ensure it is a valid position (i.e: The cell coord is less than the cell_block_dim)
  normalise_cell_coord(universe, &cell_block_position, &cell_coord);

  if (border->type == BorderType::TORUS)
  {
    wrap_cell_position_around_torus(border, universe, &cell_block_position, &cell_coord);
  }

  // If the cell position is outside of the border, don't simulate the Cell:
  // - If using FIXED border, then these cells cannot be simulated and act as the "border-buffer"
  // - If using TORUS border, then this condition should never be true, and there is a bug in
  //     wrap_cell_position_around_torus.

  b32 outside_border = !within_border(*border, cell_block_position, cell_coord);

  if (outside_border && border->type == BorderType::FIXED)
  {
    result = false;
  }
  else if (outside_border && border->type == BorderType::TORUS)
  {
    print("Error: Attempting to use cell outside border whilst in TORUS mode.\n");
    assert(0);
    result = false;
  }
  else
  {
    CellBlock *cell_block = get_existing_cell_block(universe, cell_block_position);

    // If the CellBlock doesn't exist, we will assume it contains only NULL state Cell%s.  This
    //   assumption is valid because we ensure (in create_any_new_cell_blocks_needed() ) that all
    //   Cell%s within the neighbourhood region of any neighbouring CellBlock%s are NULL Cell%s
    //   (Which don't interact with other NULL Cell's), otherwise we create the neighbouring
    //   CellBlock.

    if (cell_block != 0)
    {
      Cell *cell = get_cell_from_block(universe, cell_block, cell_coord);
      if (cell->previous_state != DEBUG_STATE)
      {
        *resulting_state = cell->previous_state;
      }
    }
  }

  return result;
}



#include "simulate.h"

#include "print.h"
#include "types.h"
#include "cell-storage.h"
#include "cell-block-coordinate-system.h"


/// @file
/// @brief Contains functions for running the CA simulation on the CellBlocks.


void
wrap_cell_position_around_torus(SimulateOptions *simulate_options, s32vec2 *cell_block_position, s32vec2 *cell_position)
{
  s32 wrapped_cell_block_x = cell_block_position->x;
  s32 wrapped_cell_block_y = cell_block_position->y;
  s32 wrapped_cell_x = cell_position->x;
  s32 wrapped_cell_y = cell_position->y;

  s32vec2 delta_block = vec2_subtract(simulate_options->border_max_corner_block, simulate_options->border_min_corner_block);
  s32vec2 delta_cell = vec2_subtract(simulate_options->border_max_corner_cell, simulate_options->border_min_corner_cell);

  // Wrap down to minimum bound

  if (cell_position_greater_than_or_equal_to(cell_block_position->x, cell_position->x,
                                             simulate_options->border_max_corner_block.x, simulate_options->border_max_corner_cell.x))
  {
    wrapped_cell_block_x = cell_block_position->x - delta_block.x;
    wrapped_cell_x = cell_position->x - delta_cell.x;

    normalise_cell_coord(&wrapped_cell_block_x, &wrapped_cell_x);
  }

  if (cell_position_greater_than_or_equal_to(cell_block_position->y, cell_position->y,
                                             simulate_options->border_max_corner_block.y, simulate_options->border_max_corner_cell.y))
  {
    wrapped_cell_block_y = cell_block_position->y - delta_block.y;
    wrapped_cell_y = cell_position->y - delta_cell.y;

    normalise_cell_coord(&wrapped_cell_block_y, &wrapped_cell_y);
  }

  // Wrap up to maximum bound

  if (cell_position_less_than(cell_block_position->x, cell_position->x,
                              simulate_options->border_min_corner_block.x, simulate_options->border_min_corner_cell.x))
  {
    wrapped_cell_block_x = cell_block_position->x + delta_block.x;
    wrapped_cell_x = cell_position->x + delta_cell.x;

    normalise_cell_coord(&wrapped_cell_block_y, &wrapped_cell_y);
  }

  if (cell_position_less_than(cell_block_position->y, cell_position->y,
                              simulate_options->border_min_corner_block.y, simulate_options->border_min_corner_cell.y))
  {
    wrapped_cell_block_y = cell_block_position->y + delta_block.y;
    wrapped_cell_y = cell_position->y + delta_cell.y;

    normalise_cell_coord(&wrapped_cell_block_y, &wrapped_cell_y);
  }

  *cell_block_position = (s32vec2){wrapped_cell_block_x, wrapped_cell_block_y};
  *cell_position = (s32vec2){wrapped_cell_x, wrapped_cell_y};
}


/// @brief Placeholder function implementing a hard coded transition rule for a single Cell.
///
/// Operates directly on the Cell in the CellBlock.
void
test_transition_rule(SimulateOptions *simulate_options, Universe *universe, CellBlock *cell_block, s32 cell_x, s32 cell_y)
{
  // Test Von Neumann neighbourhood

  s32vec2 cell_north_coord      = {cell_x,     cell_y - 1};
  s32vec2 cell_east_coord       = {cell_x + 1, cell_y};
  s32vec2 cell_south_coord      = {cell_x,     cell_y + 1};
  s32vec2 cell_west_coord       = {cell_x - 1, cell_y};
  s32vec2 cell_north_east_coord = {cell_x + 1, cell_y - 1};
  s32vec2 cell_south_east_coord = {cell_x + 1, cell_y + 1};
  s32vec2 cell_south_west_coord = {cell_x - 1, cell_y + 1};
  s32vec2 cell_north_west_coord = {cell_x - 1, cell_y - 1};

  s32vec2 cell_north_block_position      = cell_block->block_position;
  s32vec2 cell_east_block_position       = cell_block->block_position;
  s32vec2 cell_south_block_position      = cell_block->block_position;
  s32vec2 cell_west_block_position       = cell_block->block_position;
  s32vec2 cell_north_east_block_position = cell_block->block_position;
  s32vec2 cell_south_east_block_position = cell_block->block_position;
  s32vec2 cell_south_west_block_position = cell_block->block_position;
  s32vec2 cell_north_west_block_position = cell_block->block_position;

  normalise_cell_coord(&cell_north_block_position,      &cell_north_coord);
  normalise_cell_coord(&cell_east_block_position,       &cell_east_coord);
  normalise_cell_coord(&cell_south_block_position,      &cell_south_coord);
  normalise_cell_coord(&cell_west_block_position,       &cell_west_coord);
  normalise_cell_coord(&cell_north_east_block_position, &cell_north_east_coord);
  normalise_cell_coord(&cell_south_east_block_position, &cell_south_east_coord);
  normalise_cell_coord(&cell_south_west_block_position, &cell_south_west_coord);
  normalise_cell_coord(&cell_north_west_block_position, &cell_north_west_coord);

  if (simulate_options->border_type == BorderType::TORUS)
  {
    wrap_cell_position_around_torus(simulate_options, &cell_north_block_position, &cell_north_coord);
    wrap_cell_position_around_torus(simulate_options, &cell_east_block_position, &cell_east_coord);
    wrap_cell_position_around_torus(simulate_options, &cell_south_block_position, &cell_south_coord);
    wrap_cell_position_around_torus(simulate_options, &cell_west_block_position, &cell_west_coord);
    wrap_cell_position_around_torus(simulate_options, &cell_north_east_block_position, &cell_north_east_coord);
    wrap_cell_position_around_torus(simulate_options, &cell_south_east_block_position, &cell_south_east_coord);
    wrap_cell_position_around_torus(simulate_options, &cell_south_west_block_position, &cell_south_west_coord);
    wrap_cell_position_around_torus(simulate_options, &cell_north_west_block_position, &cell_north_west_coord);
  }

  CellBlock *cell_north_block      = get_cell_block(universe, cell_north_block_position);
  CellBlock *cell_east_block       = get_cell_block(universe, cell_east_block_position);
  CellBlock *cell_south_block      = get_cell_block(universe, cell_south_block_position);
  CellBlock *cell_west_block       = get_cell_block(universe, cell_west_block_position);
  CellBlock *cell_north_east_block = get_cell_block(universe, cell_north_east_block_position);
  CellBlock *cell_south_east_block = get_cell_block(universe, cell_south_east_block_position);
  CellBlock *cell_south_west_block = get_cell_block(universe, cell_south_west_block_position);
  CellBlock *cell_north_west_block = get_cell_block(universe, cell_north_west_block_position);

  Cell *cell_north      = get_cell_from_block(cell_north_block,      cell_north_coord);
  Cell *cell_east       = get_cell_from_block(cell_east_block,       cell_east_coord);
  Cell *cell_south      = get_cell_from_block(cell_south_block,      cell_south_coord);
  Cell *cell_west       = get_cell_from_block(cell_west_block,       cell_west_coord);
  Cell *cell_north_east = get_cell_from_block(cell_north_east_block, cell_north_east_coord);
  Cell *cell_south_east = get_cell_from_block(cell_south_east_block, cell_south_east_coord);
  Cell *cell_south_west = get_cell_from_block(cell_south_west_block, cell_south_west_coord);
  Cell *cell_north_west = get_cell_from_block(cell_north_west_block, cell_north_west_coord);

  Cell *cell = get_cell_from_block(cell_block, (s32vec2){cell_x, cell_y});

  u32 n_enabled_neighbours = (cell_north->previous_state + cell_east->previous_state + cell_south->previous_state + cell_west->previous_state +
                              cell_north_east->previous_state + cell_south_east->previous_state + cell_south_west->previous_state + cell_north_west->previous_state +
                              cell->previous_state);

#ifdef CA_TYPE_GROWTH
  if (n_enabled_neighbours == 3 ||
      n_enabled_neighbours == 7 ||
      n_enabled_neighbours == 8)
  {
    cell->state = 1;
  }
#else
  if (n_enabled_neighbours <= 4)
  {
    cell->state = 0;
  }
  else
  {
    cell->state = 1;
  }
#endif
}


/// Simulates one frame of a CellBlock using test_transition_rule(). Also implements the CA bounds
///   check.
void
simulate_cell_block(SimulateOptions *simulate_options, Universe *universe, CellBlock *cell_block)
{
  // print("Simulating CellBlock %d %d\n", cell_block->block_position.x, cell_block->block_position.y);

  for (s32 cell_y = 0;
       cell_y < CELL_BLOCK_DIM;
       ++cell_y)
  {
    for (s32 cell_x = 0;
         cell_x < CELL_BLOCK_DIM;
         ++cell_x)
    {
      // Bounds check
      switch (simulate_options->border_type)
      {
        case (BorderType::FIXED):
        case (BorderType::TORUS):
        {
          if (cell_position_greater_than_or_equal_to(cell_block->block_position, (s32vec2){cell_x, cell_y},
                                                     simulate_options->border_min_corner_block, simulate_options->border_min_corner_cell) &&
              cell_position_less_than(cell_block->block_position, (s32vec2){cell_x, cell_y},
                                      simulate_options->border_max_corner_block, simulate_options->border_max_corner_cell))
          {
            test_transition_rule(simulate_options, universe, cell_block, cell_x, cell_y);
          }
          else
          {
            // print("Hit boundary at %d %d\n", cell_block->block_position.x, cell_block->block_position.y);
          }
        } break;

        case (BorderType::INFINITE):
        {
          print("BorderType::INFINITE Not currently implemented.\n");
        } break;
      }
    }
  }
}


/// Simulates one frame of the Universe.
///
/// @param[in] universe
/// @param[in] current_frame  the time in us of the current frame.
///
/// First we iterate over every Cell in the Universe, copying its state into `Cell.previous_state`.
///   Next we iterate over every CellBlock and simulate it using simulate_cell_block() if it has not
///   yet been simulated on this frame. We continue to iterate over all the CellBlocks until they
///   have all been simulated on this frame; this is to allow for new CellBlocks to be created
///   during the simulation of a CellBlock, and the new CellBlock will still be simulated within the
///   same frame.
void
simulate_cells(SimulateOptions *simulate_options, Universe *universe, u64 current_frame)
{
  // First copy all Cell states into previous_state

  for (u32 hash_slot = 0;
       hash_slot < universe->hashmap_size;
       ++hash_slot)
  {
    CellBlock *cell_block = universe->hashmap[hash_slot];

    if (cell_block != 0 && cell_block->initialised)
    {
      do
      {
        for (u32 cell_index = 0;
             cell_index < CELL_BLOCK_DIM * CELL_BLOCK_DIM;
             ++cell_index)
        {
          Cell *cell = cell_block->cells + cell_index;

          cell->previous_state = cell->state;
        }

        // Follow the hashmap collision chain
        cell_block = cell_block->next_block;
      }
      while (cell_block != 0);
    }
  }

  // Loop through all CellBlocks, simulating them until they are all flagged as being
  //  simulated on this frame. This is so that new CellBlocks - which have been created
  //  when the simulation reaches the edge of an existing CellBlock - are simulated in
  //  the same frame.

  b32 simulated_any_blocks = true;
  while (simulated_any_blocks)
  {
    simulated_any_blocks = false;
    // print("Hashmap loop\n");

    for (u32 hash_slot = 0;
         hash_slot < universe->hashmap_size;
         ++hash_slot)
    {
      CellBlock *cell_block = universe->hashmap[hash_slot];

      if (cell_block != 0 &&
          cell_block->initialised)
      {
        do
        {
          if (cell_block->initialised &&
              cell_block->last_simulated_on_frame != current_frame)
          {
            cell_block->last_simulated_on_frame = current_frame;
            simulated_any_blocks = true;

            simulate_cell_block(simulate_options, universe, cell_block);
          }

          // Follow any hashmap collision chains
          cell_block = cell_block->next_block;
        }
        while (cell_block != 0);
      }
    }
  }
}
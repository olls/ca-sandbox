#include "simulate.h"

#include "print.h"
#include "types.h"
#include "cell-storage.h"
#include "cell-block-coordinate-system.h"


/// @file
/// @brief Contains functions for running the CA simulation on the CellBlocks.


/// @brief Placeholder function implementing a hard coded transition rule for a single Cell.
///
/// Operates directly on the Cell in the CellBlock.
void
test_transition_rule(Universe *universe, CellBlock *cell_block, s32 cell_x, s32 cell_y)
{
  Cell *cell = get_cell_relative_to_block(universe, cell_block, cell_x, cell_y);

  // Test Von Neumann neighbourhood
  Cell *cell_north = get_cell_relative_to_block(universe, cell_block, cell_x, cell_y - 1);
  Cell *cell_east  = get_cell_relative_to_block(universe, cell_block, cell_x + 1, cell_y);
  Cell *cell_south = get_cell_relative_to_block(universe, cell_block, cell_x, cell_y + 1);
  Cell *cell_west  = get_cell_relative_to_block(universe, cell_block, cell_x - 1, cell_y);

  Cell *cell_north_east = get_cell_relative_to_block(universe, cell_block, cell_x + 1, cell_y - 1);
  Cell *cell_south_east = get_cell_relative_to_block(universe, cell_block, cell_x + 1, cell_y + 1);
  Cell *cell_south_west = get_cell_relative_to_block(universe, cell_block, cell_x - 1, cell_y + 1);
  Cell *cell_north_west = get_cell_relative_to_block(universe, cell_block, cell_x - 1, cell_y - 1);

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
simulate_cell_block(Universe *universe, CellBlock *cell_block)
{
  // print("Simulating CellBlock %d %d\n", cell_block->block_position.x, cell_block->block_position.y);

  s32vec2 small_global_cell_upper_bound = { 137,  75};
  s32vec2 small_global_cell_lower_bound = {-137, -75};

  s32vec2 sim_upper_bounds_block;
  s32vec2 sim_upper_bounds_local_cell;
  small_global_cell_coord_to_cell_block_coords(small_global_cell_upper_bound, &sim_upper_bounds_block, &sim_upper_bounds_local_cell);

  s32vec2 sim_lower_bounds_block;
  s32vec2 sim_lower_bounds_local_cell;
  small_global_cell_coord_to_cell_block_coords(small_global_cell_lower_bound, &sim_lower_bounds_block, &sim_lower_bounds_local_cell);

  // print("Cell block bounds: (%d %d) -> (%d %d)\n", sim_lower_bounds_block.x, sim_lower_bounds_block.y, sim_upper_bounds_block.x, sim_upper_bounds_block.y);
  // print("      Cell bounds: (%d %d) -> (%d %d)\n", sim_lower_bounds_local_cell.x, sim_lower_bounds_local_cell.y, sim_upper_bounds_local_cell.x, sim_upper_bounds_local_cell.y);

  for (s32 cell_y = 0;
       cell_y < CELL_BLOCK_DIM;
       ++cell_y)
  {
    for (s32 cell_x = 0;
         cell_x < CELL_BLOCK_DIM;
         ++cell_x)
    {
      // Bounds check
      if (cell_position_greater_than_or_equal_to(cell_block->block_position, (s32vec2){cell_x, cell_y},
                                                 sim_lower_bounds_block, sim_lower_bounds_local_cell) &&
          cell_position_less_than(cell_block->block_position, (s32vec2){cell_x, cell_y},
                                  sim_upper_bounds_block, sim_upper_bounds_local_cell))
      {
        test_transition_rule(universe, cell_block, cell_x, cell_y);
      }
      else
      {
        // print("Hit boundary at %d %d\n", cell_block->block_position.x, cell_block->block_position.y);
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
simulate_cells(Universe *universe, u64 current_frame)
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

            simulate_cell_block(universe, cell_block);
          }

          // Follow any hashmap collision chains
          cell_block = cell_block->next_block;
        }
        while (cell_block != 0);
      }
    }
  }
}
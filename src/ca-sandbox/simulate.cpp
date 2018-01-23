#include "simulate.h"

#include "print.h"
#include "types.h"
#include "assert.h"
#include "allocate.h"
#include "universe.h"
#include "cell-block-coordinate-system.h"
#include "rule.h"
#include "border.h"

/// @file
/// @brief Contains functions for running the CA simulation on the CellBlock%s.


SimulateOptions
default_simulation_options()
{
  SimulateOptions result;

  result.border.type = BorderType::TORUS;

  result.border.min_corner_block = {0, 0};
  result.border.min_corner_cell = {0, 0};

  result.border.max_corner_block = {10, 10};
  result.border.max_corner_cell = {0, 0};

  return result;
}


/// Simulates one frame of a CellBlock using execute_transision_function(). Also implements the CA
///   bounds check.
void
simulate_cell_block(SimulateOptions *simulate_options, CellInitialisationOptions *cell_initialisation_options, Rule *rule, Universe *universe, CellBlock *cell_block)
{
  s32vec2 cell_position;
  for (cell_position.y = 0;
       cell_position.y < universe->cell_block_dim;
       ++cell_position.y)
  {
    for (cell_position.x = 0;
         cell_position.x < universe->cell_block_dim;
         ++cell_position.x)
    {
      if (check_border(simulate_options->border, cell_block->block_position, cell_position))
      {
        u32 subject_cell_index = get_cell_index_in_block(universe, cell_position);
        CellState *subject_cell_state = cell_block->cell_states + subject_cell_index;
        *subject_cell_state = execute_transition_function(&simulate_options->border, universe, rule, cell_block->block_position, cell_position);
      }
    }
  }
}


/// Creates new CellBlock%s around the passed in CellBlock IF:
/// - If simulate_options->boder_type == FIXED or TORUS and the new CellBlock would contain Cells
///     within the borders defined in simulate_options.
///   - OR If simulate_options->border.type == INFINITE
///     - Only create new CellBlock%s if an existing Cell __with a non-NULL state__ is within the
///         neighbourhood-region of any of its cells.
b32
create_any_new_cell_blocks_needed(SimulateOptions *simulate_options, CellInitialisationOptions *cell_initialisation_options, RuleConfiguration *rule_configuration, Universe *universe, CellBlock *subject_cell_block)
{
  b32 result = false;

  s32vec2 cell_position;
  for (cell_position.y = 0;
       cell_position.y < universe->cell_block_dim;
       ++cell_position.y)
  {
    for (cell_position.x = 0;
         cell_position.x < universe->cell_block_dim;
         ++cell_position.x)
    {
      u32 cell_index = get_cell_index_in_block(universe, cell_position);
      CellState cell_state = subject_cell_block->cell_states[cell_index];

      if (!is_null_state(rule_configuration, cell_state) &&
          check_border(simulate_options->border, subject_cell_block->block_position, cell_position))
      {
        // If within the neighbourhood region of any neighbouring CellBlocks:
        // Create the CellBlock which can see this Cell.

        b32 north_needed = cell_position.y < rule_configuration->neighbourhood_region_size;
        b32 east_needed = cell_position.x >= universe->cell_block_dim - rule_configuration->neighbourhood_region_size;
        b32 south_needed = cell_position.y >= universe->cell_block_dim - rule_configuration->neighbourhood_region_size;
        b32 west_needed = cell_position.x < rule_configuration->neighbourhood_region_size;

        if (north_needed)
        {
          s32vec2 cell_block_position = vec2_add(subject_cell_block->block_position, {0, -1});
          CellBlock *new_block = create_cell_block(universe, cell_initialisation_options, cell_block_position);
          result |= new_block != 0;
        }

        if (east_needed)
        {
          s32vec2 cell_block_position = vec2_add(subject_cell_block->block_position, {1, 0});
          CellBlock *new_block = create_cell_block(universe, cell_initialisation_options, cell_block_position);
          result |= new_block != 0;
        }

        if (south_needed)
        {
          s32vec2 cell_block_position = vec2_add(subject_cell_block->block_position, {0, 1});
          CellBlock *new_block = create_cell_block(universe, cell_initialisation_options, cell_block_position);
          result |= new_block != 0;
        }

        if (west_needed)
        {
          s32vec2 cell_block_position = vec2_add(subject_cell_block->block_position, {-1, 0});
          CellBlock *new_block = create_cell_block(universe, cell_initialisation_options, cell_block_position);
          result |= new_block != 0;
        }

        if (north_needed && east_needed)
        {
          s32vec2 cell_block_position = vec2_add(subject_cell_block->block_position, {1, -1});
          CellBlock *new_block = create_cell_block(universe, cell_initialisation_options, cell_block_position);
          result |= new_block != 0;
        }

        if (south_needed && east_needed)
        {
          s32vec2 cell_block_position = vec2_add(subject_cell_block->block_position, {1, 1});
          CellBlock *new_block = create_cell_block(universe, cell_initialisation_options, cell_block_position);
          result |= new_block != 0;
        }

        if (south_needed && west_needed)
        {
          s32vec2 cell_block_position = vec2_add(subject_cell_block->block_position, {-1, 1});
          CellBlock *new_block = create_cell_block(universe, cell_initialisation_options, cell_block_position);
          result |= new_block != 0;
        }

        if (north_needed && west_needed)
        {
          s32vec2 cell_block_position = vec2_add(subject_cell_block->block_position, {-1, -1});
          CellBlock *new_block = create_cell_block(universe, cell_initialisation_options, cell_block_position);
          result |= new_block != 0;
        }

        // Have to check the borders for torus topology as well.
        if (simulate_options->border.type == BorderType::TORUS)
        {
          // Check if the cell is within the neighbourhood_region_size of the border.

          // Upper bound
          s32vec2 max_corner_block = simulate_options->border.max_corner_block;
          s32vec2 max_corner_cell = vec2_subtract(simulate_options->border.max_corner_cell, rule_configuration->neighbourhood_region_size);
          normalise_cell_coord(universe, &max_corner_block, &max_corner_cell);

          // Lower bound
          s32vec2 min_corner_block = simulate_options->border.min_corner_block;
          s32vec2 min_corner_cell = vec2_add(simulate_options->border.min_corner_cell, rule_configuration->neighbourhood_region_size);
          normalise_cell_coord(universe, &min_corner_block, &min_corner_cell);

          b32 wrapping_north_needed = !cell_position_less_than(subject_cell_block->block_position.y, cell_position.y, max_corner_block.y, max_corner_cell.y);
          b32 wrapping_east_needed = !cell_position_greater_than_or_equal_to(subject_cell_block->block_position.x, cell_position.x, min_corner_block.x, min_corner_cell.x);
          b32 wrapping_south_needed = !cell_position_greater_than_or_equal_to(subject_cell_block->block_position.y, cell_position.y, min_corner_block.y, min_corner_cell.y);
          b32 wrapping_west_needed = !cell_position_less_than(subject_cell_block->block_position.x, cell_position.x, max_corner_block.x, max_corner_cell.x);

          if (wrapping_north_needed)
          {
            s32vec2 cell_block_position = {subject_cell_block->block_position.x, simulate_options->border.min_corner_block.y};
            CellBlock *new_block = create_cell_block(universe, cell_initialisation_options, cell_block_position);
            result |= new_block != 0;
          }

          if (wrapping_east_needed)
          {
            s32vec2 cell_block_position = {simulate_options->border.max_corner_block.x, subject_cell_block->block_position.y};
            CellBlock *new_block = create_cell_block(universe, cell_initialisation_options, cell_block_position);
            result |= new_block != 0;
          }

          if (wrapping_south_needed)
          {
            s32vec2 cell_block_position = {subject_cell_block->block_position.x, simulate_options->border.max_corner_block.y};
            CellBlock *new_block = create_cell_block(universe, cell_initialisation_options, cell_block_position);
            result |= new_block != 0;
          }

          if (wrapping_west_needed)
          {
            s32vec2 cell_block_position = {simulate_options->border.min_corner_block.x, subject_cell_block->block_position.y};
            CellBlock *new_block = create_cell_block(universe, cell_initialisation_options, cell_block_position);
            result |= new_block != 0;
          }

          if (wrapping_north_needed && wrapping_east_needed)
          {
            s32vec2 cell_block_position = {simulate_options->border.max_corner_block.x, simulate_options->border.min_corner_block.y};
            CellBlock *new_block = create_cell_block(universe, cell_initialisation_options, cell_block_position);
            result |= new_block != 0;
          }

          if (wrapping_south_needed && wrapping_east_needed)
          {
            s32vec2 cell_block_position = {simulate_options->border.max_corner_block.x, simulate_options->border.max_corner_block.y};
            CellBlock *new_block = create_cell_block(universe, cell_initialisation_options, cell_block_position);
            result |= new_block != 0;
          }

          if (wrapping_south_needed && wrapping_west_needed)
          {
            s32vec2 cell_block_position = {simulate_options->border.min_corner_block.x, simulate_options->border.max_corner_block.y};
            CellBlock *new_block = create_cell_block(universe, cell_initialisation_options, cell_block_position);
            result |= new_block != 0;
          }

          if (wrapping_north_needed && wrapping_west_needed)
          {
            s32vec2 cell_block_position = {simulate_options->border.min_corner_block.x, simulate_options->border.max_corner_block.y};
            CellBlock *new_block = create_cell_block(universe, cell_initialisation_options, cell_block_position);
            result |= new_block != 0;
          }
        }
      }
    }
  }

  return result;
}


/// Simulates one frame of the Universe.
///
/// @param[in] universe
/// @param[in] current_frame  a unique id for the current simulation step.
///
/// First we iterate over all the CellBlock%s, copying the cell_states to cell_previous_states and
///   creating any new CellBlocks needed at the edges of the simulation.
/// Then we do a second iteration over all the CellBlock%s, simulating each one.
///
void
simulate_cells(SimulateOptions *simulate_options, CellInitialisationOptions *cell_initialisation_options, Rule *rule, Universe *universe, u64 current_frame)
{
  // First copy all Cell states into previous_state
  // Then initialise any new CellBlock%s needed.

  b32 created_new_blocks = true;
  while (created_new_blocks)
  {
    created_new_blocks = false;

    for (u32 hash_slot = 0;
         hash_slot < universe->hashmap_size;
         ++hash_slot)
    {
      CellBlock *cell_block = universe->hashmap[hash_slot];

      if (cell_block != 0 && cell_block->slot_in_use)
      {
        // Follow the hashmap collision chain
        do
        {
          // Copy cell_states to cell_previous_states
          memcpy(cell_block->cell_previous_states, cell_block->cell_states, cell_block_states_array_size(universe));

          created_new_blocks |= create_any_new_cell_blocks_needed(simulate_options, cell_initialisation_options, &rule->config, universe, cell_block);

          cell_block = cell_block->next_block;
        }
        while (cell_block != 0);
      }
    }
  }

  // Simulate all CellBlock%s

  for (u32 hash_slot = 0;
       hash_slot < universe->hashmap_size;
       ++hash_slot)
  {
    CellBlock *cell_block = universe->hashmap[hash_slot];

    if (cell_block != 0 &&
        cell_block->slot_in_use)
    {
      do
      {
        if (cell_block->slot_in_use &&
            cell_block->last_simulated_on_frame != current_frame)
        {
          cell_block->last_simulated_on_frame = current_frame;

          simulate_cell_block(simulate_options, cell_initialisation_options, rule, universe, cell_block);
        }

        // Follow any hashmap collision chains
        cell_block = cell_block->next_block;
      }
      while (cell_block != 0);
    }
  }
}
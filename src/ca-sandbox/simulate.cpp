#include "ca-sandbox/simulate.h"

#include "engine/print.h"
#include "engine/types.h"
#include "engine/assert.h"
#include "engine/allocate.h"

#include "ca-sandbox/cell-blocks.h"
#include "ca-sandbox/cell-block-coordinate-system.h"
#include "ca-sandbox/rule.h"
#include "ca-sandbox/border.h"

/// @file
/// @brief Contains functions for running the CA simulation on the CellBlock%s.


SimulateOptions
default_simulation_options()
{
  SimulateOptions result;

  result.border.type = BorderType::INFINITE;

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


b32
null_state_in_block(RuleConfiguration *rule_configuration, Universe *universe, CellBlock *cell_block, s32vec2 cell_start_region, s32vec2 cell_end_region)
{
  b32 result = false;

  s32vec2 cell_position;
  for (cell_position.y = cell_start_region.y;
       cell_position.y < cell_end_region.y;
       ++cell_position.y)
  {
    for (cell_position.x = cell_start_region.x;
         cell_position.x < cell_end_region.x;
         ++cell_position.x)
    {
      u32 cell_index = get_cell_index_in_block(universe, cell_position);
      CellState cell_state = cell_block->cell_states[cell_index];

      if (!is_null_state(rule_configuration, cell_state))
      {
        result = true;
        break;
      }
    }

    if (result)
    {
      break;
    }
  }

  return result;
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

  Border& border = simulate_options->border;

  // Only check if the subject_cell_block is within the simulation border (<= because this is
  //   checking the block positions, and the border cell position is the actual bound.)
  if (border.type == BorderType::INFINITE ||
      (border.min_corner_block.x <= subject_cell_block->block_position.x &&
       border.max_corner_block.x >= subject_cell_block->block_position.x &&
       border.min_corner_block.y <= subject_cell_block->block_position.y &&
       border.max_corner_block.y >= subject_cell_block->block_position.y))
  {
    // Check the neighbourhood region size of each edge and corner of the block for non-null cell
    //   states.  If within the neighbourhood region of any neighbouring CellBlocks:  create the
    //   CellBlock which can see this Cell.

    s32 neighbourhood_region_size = s32(rule_configuration->neighbourhood_region_size);
    s32 cell_block_dim = s32(universe->cell_block_dim);

    s32 block_size_minus_neighbourhood = cell_block_dim - neighbourhood_region_size;

    s32vec2 west_start_test_region  = {0, 0};
    s32vec2 west_end_test_region    = {neighbourhood_region_size, cell_block_dim};

    s32vec2 east_start_test_region  = {block_size_minus_neighbourhood, 0};
    s32vec2 east_end_test_region    = {cell_block_dim, cell_block_dim};

    s32vec2 north_start_test_region = {0, 0};
    s32vec2 north_end_test_region   = {cell_block_dim, neighbourhood_region_size};

    s32vec2 south_start_test_region = {0, block_size_minus_neighbourhood};
    s32vec2 south_end_test_region   = {cell_block_dim, cell_block_dim};

    s32vec2 north_west_start_test_region = vec2_max(north_start_test_region, west_start_test_region);
    s32vec2 north_west_end_test_region   = vec2_min(north_end_test_region, west_end_test_region);

    s32vec2 north_east_start_test_region = vec2_max(north_start_test_region, east_start_test_region);
    s32vec2 north_east_end_test_region   = vec2_min(north_end_test_region, east_end_test_region);

    s32vec2 south_west_start_test_region = vec2_max(south_start_test_region, west_start_test_region);
    s32vec2 south_west_end_test_region   = vec2_min(south_end_test_region, west_end_test_region);

    s32vec2 south_east_start_test_region = vec2_max(south_start_test_region, east_start_test_region);
    s32vec2 south_east_end_test_region   = vec2_min(south_end_test_region, east_end_test_region);

    b32 west_needed  = null_state_in_block(rule_configuration, universe, subject_cell_block, west_start_test_region, west_end_test_region);
    b32 east_needed  = null_state_in_block(rule_configuration, universe, subject_cell_block, east_start_test_region, east_end_test_region);
    b32 north_needed = null_state_in_block(rule_configuration, universe, subject_cell_block, north_start_test_region, north_end_test_region);
    b32 south_needed = null_state_in_block(rule_configuration, universe, subject_cell_block, south_start_test_region, south_end_test_region);

    b32 north_west_needed = null_state_in_block(rule_configuration, universe, subject_cell_block, north_west_start_test_region, north_west_end_test_region);
    b32 north_east_needed = null_state_in_block(rule_configuration, universe, subject_cell_block, north_east_start_test_region, north_east_end_test_region);
    b32 south_west_needed = null_state_in_block(rule_configuration, universe, subject_cell_block, south_west_start_test_region, south_west_end_test_region);
    b32 south_east_needed = null_state_in_block(rule_configuration, universe, subject_cell_block, south_east_start_test_region, south_east_end_test_region);

    if (west_needed)
    {
      s32vec2 cell_block_position = vec2_add(subject_cell_block->block_position, {-1, 0});
      result |= create_cell_block(universe, cell_initialisation_options, cell_block_position) != 0;
    }
    if (east_needed)
    {
      s32vec2 cell_block_position = vec2_add(subject_cell_block->block_position, {1, 0});
      result |= create_cell_block(universe, cell_initialisation_options, cell_block_position) != 0;
    }
    if (north_needed)
    {
      s32vec2 cell_block_position = vec2_add(subject_cell_block->block_position, {0, -1});
      result |= create_cell_block(universe, cell_initialisation_options, cell_block_position) != 0;
    }
    if (south_needed)
    {
      s32vec2 cell_block_position = vec2_add(subject_cell_block->block_position, {0, 1});
      result |= create_cell_block(universe, cell_initialisation_options, cell_block_position) != 0;
    }
    if (south_west_needed)
    {
      s32vec2 cell_block_position = vec2_add(subject_cell_block->block_position, {-1, 1});
      result |= create_cell_block(universe, cell_initialisation_options, cell_block_position) != 0;
    }
    if (north_west_needed)
    {
      s32vec2 cell_block_position = vec2_add(subject_cell_block->block_position, {-1, -1});
      result |= create_cell_block(universe, cell_initialisation_options, cell_block_position) != 0;
    }
    if (north_east_needed)
    {
      s32vec2 cell_block_position = vec2_add(subject_cell_block->block_position, {1, -1});
      result |= create_cell_block(universe, cell_initialisation_options, cell_block_position) != 0;
    }
    if (south_east_needed)
    {
      s32vec2 cell_block_position = vec2_add(subject_cell_block->block_position, {1, 1});
      result |= create_cell_block(universe, cell_initialisation_options, cell_block_position) != 0;
    }
  }

#if 0
  if (simulate_options->border.type == BorderType::TORUS)
  {
    // Calculate the border reduced by the neighbourhood region size to check the cell block against

    // Start border + neighbourhood_region_size
    s32vec2 start_corner_block = simulate_options->border.min_corner_block;
    s32vec2 start_corner_cell = vec2_add(simulate_options->border.min_corner_cell, rule_configuration->neighbourhood_region_size);
    normalise_cell_coord(universe, &start_corner_block, &start_corner_cell);

    // End border - neighbourhood_region_size
    s32vec2 end_corner_block = simulate_options->border.max_corner_block;
    s32vec2 end_corner_cell = vec2_subtract(simulate_options->border.max_corner_cell, rule_configuration->neighbourhood_region_size);
    normalise_cell_coord(universe, &end_corner_block, &end_corner_cell);

    // Check if the current block is outside or equal to the calculated corners
    if (subject_cell_block->block_position.x <= start_corner_block.x &&
        subject_cell_block->block_position.x >= end_corner_block.x &&
        subject_cell_block->block_position.y <= start_corner_block.y &&
        subject_cell_block->block_position.y >= end_corner_block.y)
    {
      // Now need to check whether the block contains any non-null-cells in the region between the
      //   corners calculated above and the border

    }
  }
#endif

  // Have to check the borders for torus topology as well.
  // TODO: Figure out how to remove the iteration over all cells
  if (simulate_options->border.type == BorderType::TORUS)
  {
    // Upper bound
    s32vec2 max_corner_block = simulate_options->border.max_corner_block;
    s32vec2 max_corner_cell = vec2_subtract(simulate_options->border.max_corner_cell, rule_configuration->neighbourhood_region_size);
    normalise_cell_coord(universe, &max_corner_block, &max_corner_cell);

    // Lower bound
    s32vec2 min_corner_block = simulate_options->border.min_corner_block;
    s32vec2 min_corner_cell = vec2_add(simulate_options->border.min_corner_cell, rule_configuration->neighbourhood_region_size);
    normalise_cell_coord(universe, &min_corner_block, &min_corner_cell);

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
          // Check if the cell is within the neighbourhood_region_size of the border.
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

      while (cell_block != 0)
      {
        // Copy cell_states to cell_previous_states
        memcpy(cell_block->cell_previous_states, cell_block->cell_states, cell_block_states_array_size(universe));

        created_new_blocks |= create_any_new_cell_blocks_needed(simulate_options, cell_initialisation_options, &rule->config, universe, cell_block);

        cell_block = cell_block->next_block;
      }
    }
  }

  // Simulate all CellBlock%s

  for (u32 hash_slot = 0;
       hash_slot < universe->hashmap_size;
       ++hash_slot)
  {
    CellBlock *cell_block = universe->hashmap[hash_slot];

    while (cell_block != 0)
    {
      if (cell_block->last_simulated_on_frame != current_frame)
      {
        cell_block->last_simulated_on_frame = current_frame;

        simulate_cell_block(simulate_options, cell_initialisation_options, rule, universe, cell_block);
      }

      // Follow any hashmap collision chains
      cell_block = cell_block->next_block;
    }
  }
}
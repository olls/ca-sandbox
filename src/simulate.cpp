#include "simulate.h"

#include "print.h"
#include "types.h"
#include "assert.h"
#include "allocate.h"
#include "cell-storage.h"
#include "cell-block-coordinate-system.h"
#include "rule.h"

#define DEBUG_STATE 9999

/// @file
/// @brief Contains functions for running the CA simulation on the CellBlock%s.


b32
within_border(Border border, s32vec2 cell_block_position, s32vec2 cell_position)
{
  b32 result = (cell_position_greater_than_or_equal_to(cell_block_position, cell_position,
                                                       border.min_corner_block, border.min_corner_cell) &&
                cell_position_less_than(cell_block_position, cell_position,
                                        border.max_corner_block, border.max_corner_cell));

  return result;
}


SimulateOptions
default_simulation_options()
{
  SimulateOptions result;

  result.border.type = BorderType::TORUS;

  result.border.min_corner_block = {0, 0};
  result.border.min_corner_cell = {0, 0};

  result.border.max_corner_block = {10, 10};
  result.border.max_corner_cell = {0, 0};

  result.n_null_states = 1;
  result.null_states = allocate(CellState, 1);
  result.null_states[0] = 0;

  return result;
}


void
wrap_cell_position_around_torus(SimulateOptions *simulate_options, Universe *universe, s32vec2 *cell_block_position, s32vec2 *cell_position)
{
  s32 wrapped_cell_block_x = cell_block_position->x;
  s32 wrapped_cell_block_y = cell_block_position->y;
  s32 wrapped_cell_x = cell_position->x;
  s32 wrapped_cell_y = cell_position->y;

  // Size of the simulated area
  s32vec2 delta_block = vec2_subtract(simulate_options->border.max_corner_block, simulate_options->border.min_corner_block);
  s32vec2 delta_cell = vec2_subtract(simulate_options->border.max_corner_cell, simulate_options->border.min_corner_cell);

  // Wrap down to minimum bound

  if (cell_position_greater_than_or_equal_to(cell_block_position->x, cell_position->x,
                                             simulate_options->border.max_corner_block.x, simulate_options->border.max_corner_cell.x))
  {
    wrapped_cell_block_x = cell_block_position->x - delta_block.x;
    wrapped_cell_x = cell_position->x - delta_cell.x;

    normalise_cell_coord(universe, &wrapped_cell_block_x, &wrapped_cell_x);
  }

  if (cell_position_greater_than_or_equal_to(cell_block_position->y, cell_position->y,
                                             simulate_options->border.max_corner_block.y, simulate_options->border.max_corner_cell.y))
  {
    wrapped_cell_block_y = cell_block_position->y - delta_block.y;
    wrapped_cell_y = cell_position->y - delta_cell.y;

    normalise_cell_coord(universe, &wrapped_cell_block_y, &wrapped_cell_y);
  }

  // Wrap up to maximum bound

  if (cell_position_less_than(cell_block_position->x, cell_position->x,
                              simulate_options->border.min_corner_block.x, simulate_options->border.min_corner_cell.x))
  {
    wrapped_cell_block_x = cell_block_position->x + delta_block.x;
    wrapped_cell_x = cell_position->x + delta_cell.x;

    normalise_cell_coord(universe, &wrapped_cell_block_x, &wrapped_cell_x);
  }

  if (cell_position_less_than(cell_block_position->y, cell_position->y,
                              simulate_options->border.min_corner_block.y, simulate_options->border.min_corner_cell.y))
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
/// @param[in] simulate_options
/// @param[in] universe
/// @param[in] neighbouring_cell_delta  A delta in cells from the subject_block
/// @param[in] subject_block  The position of the block containing the subject_cell
/// @param[in] subject_cell  The cell to base the delta off of
///
/// @returns Boolean indicating whether the subject cell should be simulated.
bool
get_neighbouring_cell_state(SimulateOptions *simulate_options, Universe *universe, s32vec2 neighbouring_cell_delta, s32vec2 subject_block, s32vec2 subject_cell, CellState *resulting_state)
{
  b32 result = true;

  // Calculate the absolute position of the neighbouring cell
  s32vec2 cell_coord = vec2_add(subject_cell, neighbouring_cell_delta);
  s32vec2 cell_block_position = subject_block;

  // Normalise position to ensure it is a valid position (i.e: The cell coord is less than the cell_block_dim)
  normalise_cell_coord(universe, &cell_block_position, &cell_coord);

  if (simulate_options->border.type == BorderType::TORUS)
  {
    wrap_cell_position_around_torus(simulate_options, universe, &cell_block_position, &cell_coord);
  }

  // If the cell position is outside of the border, don't simulate the Cell:
  // - If using FIXED border, then these cells cannot be simulated and act as the "border-buffer"
  // - If using TORUS border, then this condition should never be true, and there is a bug in
  //     wrap_cell_position_around_torus.

  b32 outside_border = !within_border(simulate_options->border, cell_block_position, cell_coord);

  if (outside_border && simulate_options->border.type == BorderType::FIXED)
  {
    result = false;
  }
  else if (outside_border && simulate_options->border.type == BorderType::TORUS)
  {
    print("Error: Attempting to use cell outside border whilst in TORUS mode.\n");
    assert(0);
    result = false;
  }
  else
  {
    CellBlock *cell_block = get_existing_cell_block(universe, cell_block_position);

    // If the CellBlock doesn't exist, we will assume it contains only NULL state Cell%s, and set
    //   the result to null state 0.  This assumption is valid because we ensure (in
    //   create_any_new_cell_blocks_needed() ) that all Cell%s within the neighbourhood region of
    //   any neighbouring CellBlock%s are NULL Cell%s (Which don't interact with other NULL Cell's),
    //   otherwise we create the neighbouring CellBlock.

    *resulting_state = simulate_options->null_states[0];
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


/// Generate array of all the neighbour's states needed, then execute the transition function on
///   them.
///
/// Operates directly on the Cell in the CellBlock.
void
test_transition_rule(SimulateOptions *simulate_options, CellInitialisationOptions *cell_initialisation_options, Rule *rule, Universe *universe, CellBlock *cell_block, s32 cell_x, s32 cell_y)
{
  s32vec2 subject_cell_coord = {cell_x, cell_y};
  Cell *subject_cell = get_cell_from_block(universe, cell_block, subject_cell_coord);

  u32 n_neighbours = rule->n_inputs;
  s32vec2 *neighbour_positions = allocate(s32vec2, n_neighbours);

  s32vec2 cell_centre_coord     = { 0,  0};
  s32vec2 cell_north_coord      = { 0, -1};
  s32vec2 cell_east_coord       = { 1,  0};
  s32vec2 cell_south_coord      = { 0,  1};
  s32vec2 cell_west_coord       = {-1,  0};
  s32vec2 cell_north_east_coord = { 1, -1};
  s32vec2 cell_south_east_coord = { 1,  1};
  s32vec2 cell_south_west_coord = {-1,  1};
  s32vec2 cell_north_west_coord = {-1, -1};

  switch (rule->config.neighbourhood_region_shape)
  {
    case (NeighbourhoodRegionShape::MOORE):
    {
      neighbour_positions[0] = cell_north_west_coord;
      neighbour_positions[1] = cell_north_coord;
      neighbour_positions[2] = cell_north_east_coord;
      neighbour_positions[3] = cell_west_coord;
      neighbour_positions[4] = cell_centre_coord;
      neighbour_positions[5] = cell_east_coord;
      neighbour_positions[6] = cell_south_west_coord;
      neighbour_positions[7] = cell_south_coord;
      neighbour_positions[8] = cell_south_east_coord;
    } break;
    case (NeighbourhoodRegionShape::VON_NEUMANN):
    {
      neighbour_positions[0] = cell_north_coord;
      neighbour_positions[1] = cell_west_coord;
      neighbour_positions[2] = cell_centre_coord;
      neighbour_positions[3] = cell_east_coord;
      neighbour_positions[4] = cell_south_coord;
    } break;
    case (NeighbourhoodRegionShape::ONE_DIM):
    {
      neighbour_positions[0] = cell_west_coord;
      neighbour_positions[1] = cell_centre_coord;
      neighbour_positions[2] = cell_east_coord;
    } break;
  }

  CellState *neighbour_states = allocate(CellState, n_neighbours);

  b32 simulate_cell = true;
  for (u32 neighbour_n = 0;
       neighbour_n < n_neighbours;
       ++neighbour_n)
  {
    s32vec2 this_neighbour_position = neighbour_positions[neighbour_n];
    CellState *this_neighbour_state = neighbour_states + neighbour_n;

    simulate_cell &= get_neighbouring_cell_state(simulate_options, universe, this_neighbour_position, cell_block->block_position, subject_cell_coord, this_neighbour_state);
  }

  if (!simulate_cell)
  {
    subject_cell->state = DEBUG_STATE;
  }
  else
  {
    subject_cell->state = execute_transition_function(rule, neighbour_states);
  }

  un_allocate(neighbour_states);
  un_allocate(neighbour_positions);
}


/// Simulates one frame of a CellBlock using test_transition_rule(). Also implements the CA bounds
///   check.
void
simulate_cell_block(SimulateOptions *simulate_options, CellInitialisationOptions *cell_initialisation_options, Rule *rule, Universe *universe, CellBlock *cell_block)
{
  // print("Simulating CellBlock %d %d\n", cell_block->block_position.x, cell_block->block_position.y);

  for (s32 cell_y = 0;
       cell_y < universe->cell_block_dim;
       ++cell_y)
  {
    for (s32 cell_x = 0;
         cell_x < universe->cell_block_dim;
         ++cell_x)
    {
      // Bounds check
      switch (simulate_options->border.type)
      {
        case (BorderType::FIXED):
        case (BorderType::TORUS):
        {
          // Don't simulate if we are not within the border.
          if (within_border(simulate_options->border, cell_block->block_position, (s32vec2){cell_x, cell_y}))
          {
            test_transition_rule(simulate_options, cell_initialisation_options, rule, universe, cell_block, cell_x, cell_y);
          }
        } break;

        case (BorderType::INFINITE):
        {
          test_transition_rule(simulate_options, cell_initialisation_options, rule, universe, cell_block, cell_x, cell_y);
        } break;
      }
    }
  }
}


b32
is_null_state(SimulateOptions *simulate_options, CellState state)
{
  b32 result = false;

  for (u32 null_state_index = 0;
       null_state_index < simulate_options->n_null_states;
       ++null_state_index)
  {
    CellState null_state = simulate_options->null_states[null_state_index];
    result |= state == null_state;
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

  s32vec2 cell_position;
  for (cell_position.y = 0;
       cell_position.y < universe->cell_block_dim;
       ++cell_position.y)
  {
    for (cell_position.x = 0;
         cell_position.x < universe->cell_block_dim;
         ++cell_position.x)
    {
      Cell *cell = get_cell_from_block(universe, subject_cell_block, cell_position);
      cell->previous_state = cell->state;

      if (!is_null_state(simulate_options, cell->state) &&
          within_border(simulate_options->border, subject_cell_block->block_position, cell_position))
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
/// @param[in] current_frame  the time in us of the current frame.
///
/// First we iterate over every Cell in the Universe, copying its state into `Cell.previous_state`.
///   Next we iterate over every CellBlock and simulate it using simulate_cell_block() if it has not
///   yet been simulated on this frame. We continue to iterate over all the CellBlock%s until they
///   have all been simulated on this frame; this is to allow for new CellBlock%s to be created
///   during the simulation of a CellBlock, and the new CellBlock will still be simulated within the
///   same frame.
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
          created_new_blocks |= create_any_new_cell_blocks_needed(simulate_options, cell_initialisation_options, &rule->config, universe, cell_block);

          cell_block = cell_block->next_block;
        }
        while (cell_block != 0);
      }
    }
  }

  // Loop through all CellBlock%s, simulating them until they are all flagged as being
  //  simulated on this frame. This is so that new CellBlock%s - which have been created
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
          cell_block->slot_in_use)
      {
        do
        {
          if (cell_block->slot_in_use &&
              cell_block->last_simulated_on_frame != current_frame)
          {
            cell_block->last_simulated_on_frame = current_frame;
            simulated_any_blocks = true;

            simulate_cell_block(simulate_options, cell_initialisation_options, rule, universe, cell_block);
          }

          // Follow any hashmap collision chains
          cell_block = cell_block->next_block;
        }
        while (cell_block != 0);
      }
    }
  }
}
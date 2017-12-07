#ifndef SIMULATE_H_DEF
#define SIMULATE_H_DEF

#include "cell-storage.h"
#include "cell-block-coordinate-system.h"
#include "rule.h"


/// @file
///
/// - NULL states indicate states which do not need simulating, these are used so an INFINITE border
///     simulation can still run in finite time.
///   - CellBlocks must be initialised with NULL states.
///   - NULL state rules must be stable (i.e. No change in state) when inputs in neighbourhood
///       region are also NULL states.
///   - This allows the simulator to avoid simulating/creating a CellBlock if:
///     - Its initial state is all NULL states
///     - It is still in its initial state
///     - There are no non-NULL state Cells within the rule neighbourhood of the CellBlock's border.
///


enum struct BorderType
{
  FIXED,
  INFINITE,
  TORUS
};


/// Border types:
/// - FIXED: A fixed border from border_min_corner_{block,cell} to border_max_corner_{block,cell}
/// - INFINITE: Automata expands infinitely
/// - TORUS: Wraps around from top to bottom and side to side, at the borders.
struct Border
{
  BorderType type;
  s32vec2 min_corner_block;
  s32vec2 min_corner_cell;
  s32vec2 max_corner_block;
  s32vec2 max_corner_cell;
};


struct SimulateOptions
{
  // TODO: This is duplicated in the Rule, probably should only be there.
  /// The manhattan distance from a cell to its neighbourhood region edge, e.g: a value of 1 would
  ///   mean the Cell has one one cell on all sides in its neighbourhood region
  u32 neighbourhood_region_size;

  Border border;

  /// Array of state values which are NULL states
  u32 *null_states;
  /// Length of null_states
  u32 n_null_states;
};


SimulateOptions
default_simulation_options();


void
simulate_cells(SimulateOptions *simulate_options, CellInitialisationOptions *cell_initialisation_options, Rule *rule, Universe *universe, u64 current_frame);


b32
within_border(Border border, s32vec2 cell_block_position, s32vec2 cell_position);


#endif
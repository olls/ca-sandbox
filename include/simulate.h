#ifndef SIMULATE_H_DEF
#define SIMULATE_H_DEF

#include "cell-storage.h"
#include "cell-block-coordinate-system.h"
#include "rule.h"
#include "border.h"


#define DEBUG_STATE 9999


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


struct SimulateOptions
{
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


#endif
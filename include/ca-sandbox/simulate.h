#ifndef SIMULATE_H_DEF
#define SIMULATE_H_DEF

#include "ca-sandbox/cell-blocks.h"
#include "ca-sandbox/cell-block-coordinate-system.h"
#include "ca-sandbox/rule.h"
#include "ca-sandbox/border.h"

#define DEBUG_STATE 9999

/// @file
///


struct SimulateOptions
{
  Border border;
};


SimulateOptions
default_simulation_options();


void
simulate_cells(SimulateOptions *simulate_options, CellInitialisationOptions *cell_initialisation_options, Rule *rule, Universe *universe, u64 current_frame);


#endif
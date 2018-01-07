#ifndef SIMULATION_UI_H_DEF
#define SIMULATION_UI_H_DEF

#include "types.h"
#include "text.h"

/// @file
///


enum struct Mode
{
  EDITOR,
  SIMULATOR
};


/// Misc UI variables for managing the program.
struct SimulationUI
{
  Mode mode;

  b32 simulating;
  r32 sim_frequency;
  b32 step_simulation;
};


void
do_simulation_ui(SimulationUI *simulation_ui, u32 last_simulation_delta, b32 *reload_universe);


#endif
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

  /// Currently simulating, or paused
  b32 simulating;

  /// The frequency of simulation steps
  r32 sim_frequency;

  /// Step the simulation this frame, if paused
  b32 step_simulation;

  /// The simulation step counter
  u64 simulation_step;

  /// The time of the last simulation step end
  u64 last_sim_time;

  /// The amount of time the last simulation step took
  u32 last_simulation_delta;
};


void
do_simulation_ui(SimulationUI *simulation_ui, u64 frame_start, b32 rule_tree_built, b32 *reload_universe, b32 *save_universe);


#endif
#ifndef MISC_UI_H_DEF
#define MISC_UI_H_DEF

#include "types.h"
#include "text.h"

/// @file
///


/// Misc UI variables for managing the program.
struct MiscUI
{
  r32 sim_frequency;
  b32 simulating;
  b32 step_simulation;
};


void
miscellaneous_ui(MiscUI *misc_ui, u32 last_simulation_delta);


#endif
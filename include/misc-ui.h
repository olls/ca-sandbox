#ifndef MISC_UI_H_DEF
#define MISC_UI_H_DEF

#include "types.h"
#include "text.h"


/// Misc UI variables for managing the program.
struct MiscUI
{
  r32 sim_frequency;
  b32 step_simulation;
};


void
miscellaneous_ui(MiscUI *misc_ui);


#endif
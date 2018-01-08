#ifndef SAVE_UNIVERSE_H_DEF
#define SAVE_UNIVERSE_H_DEF

#include "universe.h"
#include "simulate.h"
#include "named-states.h"


b32
save_universe_to_file(const char *filename, Universe *universe, SimulateOptions *simulate_options, NamedStates *named_states);


#endif
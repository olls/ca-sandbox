#include "cell.h"

#include "random.h"
#include "vectors.h"
#include "allocate.h"

/// @file
/// @brief  Functions for initialising a Cell struct
///


CellState
initialise_cell_state(CellInitialisationOptions *cell_initialisation_options, s32vec2 position)
{
  CellState result;

  if (cell_initialisation_options->type == CellInitialisationType::RANDOM)
  {
    result = random_u32(0, cell_initialisation_options->set_of_initial_states_size);
  }

  return result;
}


CellInitialisationOptions
default_cell_initialisation_options()
{
  CellInitialisationOptions result;

  result.type = CellInitialisationType::RANDOM;

  result.set_of_initial_states_size = 1;
  result.set_of_initial_states = allocate(CellState, result.set_of_initial_states_size);
  result.set_of_initial_states[0] = 0;

  return result;
}
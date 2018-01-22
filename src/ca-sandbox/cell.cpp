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
    u32 initial_state_index = random_u32(0, cell_initialisation_options->set_of_initial_states.n_elements);
    result = cell_initialisation_options->set_of_initial_states[initial_state_index];
  }

  return result;
}


void
default_cell_initialisation_options(CellInitialisationOptions *result)
{
  result->type = CellInitialisationType::RANDOM;

  Array::clear(result->set_of_initial_states);

  CellState zero_state = 0;
  Array::add(result->set_of_initial_states, zero_state);
}
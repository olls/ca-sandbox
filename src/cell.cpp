#include "cell.h"

#include "random.h"


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
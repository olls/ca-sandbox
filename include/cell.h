#ifndef CELL_H_DEF
#define CELL_H_DEF

#include "vectors.h"


/// @file
/// @brief Definition of Cell


typedef u32 CellState;


/// An individual cell in a CA, stored in CellBlock%s.
struct Cell
{
  CellState state;

  /// Stores the previous state, so during a simulation frame, the state value can be updated
  ///   leaving the previous state for neighbouring Cell%s to use.
  u32 previous_state;
};


enum struct CellInitialisationType
{
  RANDOM
};


struct CellInitialisationOptions
{
  CellInitialisationType type;

  CellState *set_of_initial_states;
  u32 set_of_initial_states_size;
};


CellState
initialise_cell_state(CellInitialisationOptions *cell_initialisation_options, s32vec2 position);


#endif
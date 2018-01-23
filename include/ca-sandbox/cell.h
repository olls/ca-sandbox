#ifndef CELL_H_DEF
#define CELL_H_DEF

#include "engine/vectors.h"
#include "engine/my-array.h"

/// @file
/// @brief structs for cell storage, and configuration
///


typedef u32 CellState;


enum struct CellInitialisationType : u32
{
  RANDOM
};


static const char *CELL_INITIALISATION_TYPE_STRINGS[] = {
  "RANDOM"
};


struct CellInitialisationOptions
{
  CellInitialisationType type;

  Array::Array<CellState> set_of_initial_states;
};


CellState
initialise_cell_state(CellInitialisationOptions *cell_initialisation_options, s32vec2 position);


void
default_cell_initialisation_options(CellInitialisationOptions *result);


#endif
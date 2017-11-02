#ifndef CELL_H_DEF
#define CELL_H_DEF

#include "vectors.h"


/// @file
/// @brief Definition of Cell


/// An individual cell in a CA, stored in CellBlock%s.
struct Cell
{
  /// Integer position relative to the block
  uvec2 block_offset;

  u32 state;

  /// Stores the previous state, so during a simulation frame, the state value can be updated
  ///   leaving the previous state for neighbouring Cell%s to use.
  u32 previous_state;
};


#endif
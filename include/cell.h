#ifndef CELL_H_DEF
#define CELL_H_DEF

#include "vectors.h"


struct Cell
{
  // Integer position relative to the block
  uvec2 block_offset;

  u32 state;
};


#endif
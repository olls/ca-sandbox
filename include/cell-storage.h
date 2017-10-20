#ifndef CELL_STORAGE_H_DEF
#define CELL_STORAGE_H_DEF

#include "cell.h"
#include "vectors.h"


const u32 CELL_BLOCK_DIM = 16;

struct CellBlock
{
  b32 initalised;

  s64Vec2 block_position;

  Cell cells[CELL_BLOCK_DIM * CELL_BLOCK_DIM];

  CellBlock *next_block;
};


const u32 INITIAL_CELL_HASHMAP_SIZE = 32;

struct Universe
{
  CellBlock **hashmap;
  u32 hashmap_size;
};


void
init_cell_hashmap(Universe *universe);


CellBlock *
get_cell_block(Universe *universe, s64Vec2 search_cell_block_position);


#endif
#ifndef CELL_STORAGE_H_DEF
#define CELL_STORAGE_H_DEF


// - CellBlocks store NxN block of cells
// - Universe is made up of a grid of CellBlocks
// - All cells in a CellBlock are initialised
// - CellBlocks are stored on the heap, and accessed via a hashmap on the block position.
//   - i.e.: block_pos = cell.pos / block_size
// - On simulation, each cell block is simulated as a whole
// - To iterate over all CellBlocks / Cells just loop through the hashmap.
// - Possible optimisation: CellBlocks store pointers to neighbours for quick access to border cell states.


#include "cell.h"
#include "vectors.h"


const u32 CELL_BLOCK_DIM = 16;

struct CellBlock
{
  b32 initialised;

  s32vec2 block_position;

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
get_cell_block(Universe *universe, s32vec2 search_cell_block_position);


#endif
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "cell.h"
#include "cell-storage.h"
#include "types.h"


s64Vec2
cell_position_to_cell_block_position(s64Vec2 cell_position)
{
  s64Vec2 result = cell_position / CELL_BLOCK_DIM;

  return result;
}


s64Vec2
cell_block_position_to_cell_position(s64Vec2 cell_block_position)
{
  s64Vec2 result = cell_block_position * CELL_BLOCK_DIM;

  return result;
}


void
init_cell_hashmap(Universe *universe)
{
  universe->hashmap_size = INITIAL_CELL_HASHMAP_SIZE;
  universe->hashmap = (CellBlock **)malloc(universe->hashmap_size * sizeof(CellBlock *));
  memset(universe->hashmap, 0, universe->hashmap_size * sizeof(CellBlock *));
}


void
init_cell_block(CellBlock *cell_block, s64Vec2 pos)
{
  memset(cell_block, 0, sizeof(CellBlock));

  cell_block->block_position = pos;
  cell_block->initalised = true;

  // TODO: Initialise cells
}


CellBlock *
get_cell_block(Universe *universe, s64Vec2 search_cell_block_position)
{
  printf("Getting CellBlock (%ld, %ld).\n", search_cell_block_position.x, search_cell_block_position.y);
  CellBlock *result = 0;

  u32 cell_block_hash = search_cell_block_position.x * 7 + search_cell_block_position.y * 13;
  cell_block_hash %= universe->hashmap_size;

  CellBlock *candidate_cell_block = universe->hashmap[cell_block_hash];
  if (candidate_cell_block == 0)
  {
    // Hash slot was 0, allocate CellBlock
    candidate_cell_block = (CellBlock *)malloc(sizeof(CellBlock));
    init_cell_block(candidate_cell_block, search_cell_block_position);
  }

  // Follow CellBlock linked list
  while (candidate_cell_block->initalised &&
         candidate_cell_block->block_position != search_cell_block_position)
  {
    // Allocate next_block if it doesn't exist, as the CellBlock __must__ be in this hash slot.
    if (candidate_cell_block->next_block == 0)
    {
      candidate_cell_block->next_block = (CellBlock *)malloc(sizeof(CellBlock));
      init_cell_block(candidate_cell_block->next_block, search_cell_block_position);
    }

    candidate_cell_block = candidate_cell_block->next_block;
  }

  result = candidate_cell_block;

  return result;
}

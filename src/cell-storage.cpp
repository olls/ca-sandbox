#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "print.h"
#include "cell.h"
#include "cell-storage.h"


void
init_cell_hashmap(Universe *universe)
{
  universe->hashmap_size = INITIAL_CELL_HASHMAP_SIZE;
  universe->hashmap = (CellBlock **)malloc(universe->hashmap_size * sizeof(CellBlock *));
  memset(universe->hashmap, 0, universe->hashmap_size * sizeof(CellBlock *));
}


void
init_cell_block(CellBlock *cell_block, s32vec2 position)
{
  static u32 s = 0;

  print("Initialised CellBlock (%d, %d).\n", position.x, position.y);
  memset(cell_block, 0, sizeof(CellBlock));

  cell_block->block_position = position;
  cell_block->initialised = true;

  for (u32 cell_y = 0;
       cell_y < CELL_BLOCK_DIM;
       ++cell_y)
  {
    for (u32 cell_x = 0;
         cell_x < CELL_BLOCK_DIM;
         ++cell_x)
    {
      Cell *cell = cell_block->cells + (cell_y * CELL_BLOCK_DIM) + cell_x;

      cell->block_offset = (uvec2){cell_x, cell_y};
      cell->state = s++;
    }
  }
}


CellBlock *
get_cell_block(Universe *universe, s32vec2 search_cell_block_position)
{
  print("Getting CellBlock (%d, %d).\n", search_cell_block_position.x, search_cell_block_position.y);
  CellBlock *result = 0;

  u32 cell_block_hash = search_cell_block_position.x * 7 + search_cell_block_position.y * 13;
  cell_block_hash %= universe->hashmap_size;

  CellBlock *candidate_cell_block = universe->hashmap[cell_block_hash];
  if (candidate_cell_block == 0)
  {
    // Hash slot was 0, allocate CellBlock
    candidate_cell_block = (CellBlock *)malloc(sizeof(CellBlock));

    // Store the pointer in the hash map!
    universe->hashmap[cell_block_hash] = candidate_cell_block;

    init_cell_block(candidate_cell_block, search_cell_block_position);
  }

  // Follow CellBlock linked list
  while (candidate_cell_block->initialised &&
         !vec2_eq(candidate_cell_block->block_position, search_cell_block_position))
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

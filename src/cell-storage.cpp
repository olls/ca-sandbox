#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "print.h"
#include "cell.h"
#include "cell-storage.h"


/// @file
/// @brief Implements functions for accessing the Universe.


/// Initialise the universe

/// Sets the hashmap_size to INITIAL_CELL_HASHMAP_SIZE, and allocates the hashmap.
///
void
init_cell_hashmap(Universe *universe)
{
  universe->hashmap_size = INITIAL_CELL_HASHMAP_SIZE;
  universe->hashmap = (CellBlock **)malloc(universe->hashmap_size * sizeof(CellBlock *));
  memset(universe->hashmap, 0, universe->hashmap_size * sizeof(CellBlock *));
}


/// Initialise a CellBlock

/// Sets the position member of the CellBlock to the given position, and initialises all of the
///   Cell%s in the block.
/// Cell%s are initialised by giving them their position within the CellBlock, initialising their
///   state and previous_state.
///
void
init_cell_block(CellBlock *cell_block, s32vec2 position)
{
  print("Initialised CellBlock (%d, %d).\n", position.x, position.y);
  memset(cell_block, 0, sizeof(CellBlock));

  cell_block->block_position = position;
  cell_block->initialised = true;
  cell_block->last_simulated_on_frame = 0;

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
      cell->state = rand()%2;
      cell->previous_state = cell->state;
    }
  }
}


/// Returns the CellBlock from the hashmap.

/// Indexes the Universe hashmap to retrieve the CellBlock at search_cell_block_position.  Creates a
///   new CellBlock on the heap, if it doesn't exist, then initialise all of its Cell%s.
///
/// @param [in] universe                    Pointer to an initialised Universe.
/// @param [in] search_cell_block_position  The position of the CellBlock to get.
///
/// @returns 0 on error
CellBlock *
get_cell_block(Universe *universe, s32vec2 search_cell_block_position)
{
  // print("Getting CellBlock (%d, %d).\n", search_cell_block_position.x, search_cell_block_position.y);
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


/// Retrieves a cell from within 1 CELL_BLOCK_DIM of the cell_block

/// Gets a cell at a position relative to the `cell_block`; if the cell is not within the
///   `cell_block` but it is within one of the `cell_block`s eight immediate neighbours, it will
///   retrieve the neighbouring CellBlock and return the correct cell.
///
/// This is used to retrieve cells across CellBlock borders seamlessly.
///
/// @returns 0 on failure, this shouldn't happen as get_cell_block() will create new CellBlocks if
///            they don't exist.
Cell *
get_cell_relative_to_block(Universe *universe, CellBlock *cell_block, s32 cell_x, s32 cell_y)
{
  // TODO: Cache CellBlocks

  Cell *result = 0;

  s32vec2 relative_cell_block = {0, 0};

  if (cell_x < 0)
  {
    relative_cell_block.x = -1;
    cell_x += CELL_BLOCK_DIM;
  }
  if (cell_x >= CELL_BLOCK_DIM)
  {
    relative_cell_block.x = 1;
    cell_x -= CELL_BLOCK_DIM;
  }

  if (cell_y < 0)
  {
    relative_cell_block.y = -1;
    cell_y += CELL_BLOCK_DIM;
  }
  if (cell_y >= CELL_BLOCK_DIM)
  {
    relative_cell_block.y = 1;
    cell_y -= CELL_BLOCK_DIM;
  }

  if (!vec2_eq(relative_cell_block, (s32vec2){0, 0}))
  {
    s32vec2 new_cell_block_position = vec2_add(relative_cell_block, cell_block->block_position);

    CellBlock *new_cell_block = get_cell_block(universe, new_cell_block_position);
    cell_block = new_cell_block;
  }

  if (cell_block != 0)
  {
    result = cell_block->cells + (cell_y * CELL_BLOCK_DIM) + cell_x;
  }

  return result;
}
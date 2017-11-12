#include <stdlib.h>
#include <string.h>

#include "cell-storage.h"

#include "types.h"
#include "print.h"
#include "assert.h"
#include "allocate.h"
#include "cell.h"


/// @file
/// @brief Implements functions for accessing the Universe.


/// Initialise the universe

/// Sets the hashmap_size to INITIAL_CELL_HASHMAP_SIZE, and allocates the hashmap.
///
void
init_cell_hashmap(Universe *universe)
{
  universe->cell_block_dim = DEFAULT_CELL_BLOCK_DIM;
  universe->hashmap_size = INITIAL_CELL_HASHMAP_SIZE;
  universe->hashmap = allocate(CellBlock *, universe->hashmap_size);
  memset(universe->hashmap, 0, universe->hashmap_size * sizeof(CellBlock *));
}


/// Initialise a CellBlock

/// Sets the position member of the CellBlock to the given position, and initialises all of the
///   Cell%s in the block.
/// Cell%s are initialised by giving them their position within the CellBlock, initialising their
///   state and previous_state.
///
CellBlock *
init_cell_block(Universe *universe, CellInitialisationOptions *cell_initialisation_options, s32vec2 position)
{
  print("Initialised CellBlock (%d, %d).\n", position.x, position.y);

  u32 size = sizeof(CellBlock) + (sizeof(Cell) * universe->cell_block_dim * universe->cell_block_dim);
  CellBlock *result = (CellBlock *)_allocate(size, 1);
  memset(result, 0, size);

  result->block_position = position;
  result->initialised = true;
  result->last_simulated_on_frame = 0;

  for (u32 cell_y = 0;
       cell_y < universe->cell_block_dim;
       ++cell_y)
  {
    for (u32 cell_x = 0;
         cell_x < universe->cell_block_dim;
         ++cell_x)
    {
      Cell *cell = result->cells + (cell_y * universe->cell_block_dim) + cell_x;

      cell->state = initialise_cell_state(cell_initialisation_options, position);

      cell->previous_state = cell->state;
    }
  }

  return result;
}


/// Attempts to create the CellBlock in the heap, if it doesn't already exit.

/// Indexes the Universe hashmap to retrieve the CellBlock at search_cell_block_position.  Creates a
///   new CellBlock on the heap, if it doesn't exist, then initialise all of its Cell%s.
///
/// @param [in] universe                    Pointer to an initialised Universe.
/// @param [in] search_cell_block_position  The position of the CellBlock to get.
///
/// @returns 0 if the CellBlock already exists.
CellBlock *
create_cell_block(Universe *universe, CellInitialisationOptions *cell_initialisation_options, s32vec2 search_cell_block_position)
{
  CellBlock *result = 0;

  u32 cell_block_hash = search_cell_block_position.x * 7 + search_cell_block_position.y * 13;
  cell_block_hash %= universe->hashmap_size;

  CellBlock *candidate_cell_block = universe->hashmap[cell_block_hash];
  if (candidate_cell_block == 0)
  {
    // Hash slot was 0, allocate CellBlock
    candidate_cell_block = init_cell_block(universe, cell_initialisation_options, search_cell_block_position);
    result = candidate_cell_block;

    // Store the pointer in the hash map!
    universe->hashmap[cell_block_hash] = candidate_cell_block;
  }

  // Follow CellBlock linked list
  while (candidate_cell_block->initialised &&
         !vec2_eq(candidate_cell_block->block_position, search_cell_block_position))
  {
    // Allocate next_block if it doesn't exist, as the CellBlock __must__ be in this hash slot.
    if (candidate_cell_block->next_block == 0)
    {
      candidate_cell_block->next_block = init_cell_block(universe, cell_initialisation_options, search_cell_block_position);
      result = candidate_cell_block;
    }

    candidate_cell_block = candidate_cell_block->next_block;
  }

  return result;
}


/// Returns the CellBlock from the hashmap, creating it if necessary.

/// Indexes the Universe hashmap to retrieve the CellBlock at search_cell_block_position.  Creates a
///   new CellBlock on the heap, if it doesn't exist, then initialise all of its Cell%s.
///
/// @param [in] universe                    Pointer to an initialised Universe.
/// @param [in] search_cell_block_position  The position of the CellBlock to get.
///
/// @returns 0 on error
CellBlock *
get_or_create_cell_block(Universe *universe, CellInitialisationOptions *cell_initialisation_options, s32vec2 search_cell_block_position)
{
  // print("Getting CellBlock (%d, %d).\n", search_cell_block_position.x, search_cell_block_position.y);
  CellBlock *result = 0;

  u32 cell_block_hash = search_cell_block_position.x * 7 + search_cell_block_position.y * 13;
  cell_block_hash %= universe->hashmap_size;

  CellBlock *candidate_cell_block = universe->hashmap[cell_block_hash];
  if (candidate_cell_block == 0)
  {
    // Hash slot was 0, allocate CellBlock
    candidate_cell_block = init_cell_block(universe, cell_initialisation_options, search_cell_block_position);

    // Store the pointer in the hash map!
    universe->hashmap[cell_block_hash] = candidate_cell_block;
  }

  // Follow CellBlock linked list
  while (candidate_cell_block->initialised &&
         !vec2_eq(candidate_cell_block->block_position, search_cell_block_position))
  {
    // Allocate next_block if it doesn't exist, as the CellBlock __must__ be in this hash slot.
    if (candidate_cell_block->next_block == 0)
    {
      candidate_cell_block->next_block = init_cell_block(universe, cell_initialisation_options, search_cell_block_position);
    }

    candidate_cell_block = candidate_cell_block->next_block;
  }

  result = candidate_cell_block;

  return result;
}


/// Returns the CellBlock from the hashmap, or 0 if it doesn't exist

/// Indexes the Universe hashmap to retrieve the CellBlock at search_cell_block_position.
///
/// @param [in] universe                    Pointer to an initialised Universe.
/// @param [in] search_cell_block_position  The position of the CellBlock to get.
///
/// @returns 0 on error
CellBlock *
get_existing_cell_block(Universe *universe, s32vec2 search_cell_block_position)
{
  CellBlock *result = 0;

  u32 cell_block_hash = search_cell_block_position.x * 7 + search_cell_block_position.y * 13;
  cell_block_hash %= universe->hashmap_size;

  result = universe->hashmap[cell_block_hash];
  if (result != 0)
  {
    // Follow CellBlock linked list
    while (result->initialised &&
           !vec2_eq(result->block_position, search_cell_block_position))
    {
      result = result->next_block;

      if (result == 0)
      {
        break;
      }
    }
  }

  return result;
}


/// Get a Cell from a position within the CellBlock
Cell *
get_cell_from_block(Universe *universe, CellBlock *cell_block, s32vec2 cell_coord)
{
  Cell *result = 0;

  assert(cell_coord.x >= 0);
  assert(cell_coord.y >= 0);
  assert(cell_coord.x < universe->cell_block_dim);
  assert(cell_coord.y < universe->cell_block_dim);

  result = cell_block->cells + (cell_coord.y * universe->cell_block_dim) + cell_coord.x;

  return result;
}
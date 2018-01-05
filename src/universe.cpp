#include "universe.h"

#include "types.h"
#include "print.h"
#include "assert.h"
#include "allocate.h"
#include "cell.h"

#include <string.h>

/// @file
/// @brief Implements functions for accessing the Cells stored in the Universe.
///


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


void
destroy_cell_hashmap(Universe *universe)
{
  if (universe->hashmap != 0)
  {
    // Un-allocate all CellBlocks in the hashmap
    for (u32 slot_n = 0;
         slot_n < universe->hashmap_size;
         ++slot_n)
    {
      CellBlock *cell_block = universe->hashmap[slot_n];

      while (cell_block != 0)
      {
        CellBlock *next = cell_block->next_block;
        un_allocate(cell_block);
        cell_block = next;
      }
    }
  }

  if (universe->hashmap != 0)
  {
    un_allocate(universe->hashmap);
  }
}


CellBlock *
allocate_cell_block(Universe *universe, s32vec2 position)
{
  u32 size = sizeof(CellBlock) + (sizeof(Cell) * universe->cell_block_dim * universe->cell_block_dim);

  CellBlock *result = (CellBlock *)allocate_size(size, 1);
  memset(result, 0, size);

  result->block_position = position;
  result->slot_in_use = true;

  return result;
}


/// Initialise all cells in a CellBlock

/// Sets the position member of the CellBlock to the given position, and initialises all of the
///   Cell%s in the block.
/// Cell%s are initialised by giving them their position within the CellBlock, initialising their
///   state and previous_state.
///
void
init_cells(Universe *universe, CellInitialisationOptions *cell_initialisation_options, CellBlock *cell_block, s32vec2 position)
{
  print("Initialised CellBlock (%d, %d).\n", position.x, position.y);

  for (u32 cell_y = 0;
       cell_y < universe->cell_block_dim;
       ++cell_y)
  {
    for (u32 cell_x = 0;
         cell_x < universe->cell_block_dim;
         ++cell_x)
    {
      Cell *cell = cell_block->cells + (cell_y * universe->cell_block_dim) + cell_x;

      cell->state = initialise_cell_state(cell_initialisation_options, position);
      cell->previous_state = cell->state;
    }
  }
}


CellBlock **
get_cell_block_slot(Universe *universe, s32vec2 search_cell_block_position)
{
  CellBlock **result = 0;

  u32 cell_block_hash = search_cell_block_position.x * 7 + search_cell_block_position.y * 13;
  cell_block_hash %= universe->hashmap_size;

  CellBlock **hash_slot = universe->hashmap + cell_block_hash;

  CellBlock *candidate_cell_block = *hash_slot;
  if (candidate_cell_block == 0 ||
      vec2_eq(candidate_cell_block->block_position, search_cell_block_position))
  {
    // Slot is empty, or is the correct slot
    result = hash_slot;
  }
  else
  {
    // There already a different cell in this slot, follow the hash chain to either find our slot,
    //   or the last empty slot.

    while (candidate_cell_block != 0 &&
           candidate_cell_block->slot_in_use &&
           !vec2_eq(candidate_cell_block->block_position, search_cell_block_position))
    {
      result = &candidate_cell_block->next_block;
      candidate_cell_block = candidate_cell_block->next_block;
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

  CellBlock **cell_block_slot = get_cell_block_slot(universe, search_cell_block_position);

  if (*cell_block_slot == 0)
  {
    *cell_block_slot = allocate_cell_block(universe, search_cell_block_position);
    result = *cell_block_slot;

    init_cells(universe, cell_initialisation_options, result, search_cell_block_position);
  }
  else
  {
    // CellBlock already exists.
  }

  return result;
}


CellBlock *
create_uninitialised_cell_block(Universe *universe, s32vec2 search_cell_block_position)
{
  CellBlock *result = 0;

  CellBlock **cell_block_slot = get_cell_block_slot(universe, search_cell_block_position);

  if (*cell_block_slot == 0)
  {
    *cell_block_slot = allocate_cell_block(universe, search_cell_block_position);
    result = *cell_block_slot;
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
  CellBlock *result = 0;

  CellBlock **cell_block_slot = get_cell_block_slot(universe, search_cell_block_position);

  if (*cell_block_slot == 0)
  {
    *cell_block_slot = allocate_cell_block(universe, search_cell_block_position);
    result = *cell_block_slot;

    init_cells(universe, cell_initialisation_options, result, search_cell_block_position);
  }
  else
  {
    result = *cell_block_slot;
  }

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

  CellBlock **cell_block_slot = get_cell_block_slot(universe, search_cell_block_position);

  if (*cell_block_slot != 0)
  {
    result = *cell_block_slot;
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
#include "ca-sandbox/cell-blocks.h"

#include "engine/types.h"
#include "engine/print.h"
#include "engine/assert.h"
#include "engine/allocate.h"

#include "ca-sandbox/cell.h"

#include <string.h>

/// @file
/// @brief Implements functions for accessing the Cells stored in the Universe.
///


/// Initialise the cell_blocks
///
/// Sets the hashmap_size to INITIAL_CELL_HASHMAP_SIZE, and allocates the hashmap.
///
void
init_cell_hashmap(CellBlocks *cell_blocks)
{
  cell_blocks->cell_block_dim = DEFAULT_CELL_BLOCK_DIM;
  cell_blocks->hashmap_size = INITIAL_CELL_HASHMAP_SIZE;
  cell_blocks->hashmap = allocate(CellBlock *, cell_blocks->hashmap_size);
  memset(cell_blocks->hashmap, 0, cell_blocks->hashmap_size * sizeof(CellBlock *));
}


void
destroy_cell_hashmap(CellBlocks *cell_blocks)
{
  if (cell_blocks->hashmap != 0)
  {
    // Un-allocate all CellBlocks in the hashmap
    for (u32 slot_n = 0;
         slot_n < cell_blocks->hashmap_size;
         ++slot_n)
    {
      CellBlock *cell_block = cell_blocks->hashmap[slot_n];

      while (cell_block != 0)
      {
        CellBlock *next = cell_block->next_block;
        un_allocate(cell_block);
        cell_block = next;
      }
    }
  }

  if (cell_blocks->hashmap != 0)
  {
    un_allocate(cell_blocks->hashmap);
  }
}


u32
cell_block_states_array_size(CellBlocks *cell_blocks)
{
  u32 result = sizeof(CellState) * cell_blocks->cell_block_dim * cell_blocks->cell_block_dim;
  return result;
}


u32
cell_block_size(CellBlocks *cell_blocks)
{
  // CellBlocks have 2 arrays of CellStates (cell_states, cell_previous_states) allocated beyond the
  //   struct
  u32 result = sizeof(CellBlock) + (2*cell_block_states_array_size(cell_blocks));
  return result;
}


CellBlock *
allocate_cell_block(CellBlocks *cell_blocks, s32vec2 position)
{
  u32 size = cell_block_size(cell_blocks);

  CellBlock *result = (CellBlock *)allocate_size(size, 1);
  memset(result, 0, size);

  // Calculate cell_states and cell_previous_states offsets
  result->cell_states = (CellState *)((u8*)result + sizeof(CellBlock) + cell_block_states_array_size(cell_blocks) * 0);
  result->cell_previous_states = (CellState *)((u8*)result + sizeof(CellBlock) + cell_block_states_array_size(cell_blocks) * 1);
  assert((u8 *)result->cell_previous_states + cell_block_states_array_size(cell_blocks) == (u8 *)result + size);

  result->block_position = position;

  return result;
}


/// Initialise all cells in a CellBlock

/// Sets the position member of the CellBlock to the given position, and initialises all of the
///   Cell%s in the block.
/// Cell%s are initialised by giving them their position within the CellBlock, initialising their
///   state and previous_state.
///
void
init_cells(CellBlocks *cell_blocks, CellInitialisationOptions *cell_initialisation_options, CellBlock *cell_block, s32vec2 position)
{
  print("Initialised CellBlock (%d, %d).\n", position.x, position.y);

  for (u32 cell_y = 0;
       cell_y < cell_blocks->cell_block_dim;
       ++cell_y)
  {
    for (u32 cell_x = 0;
         cell_x < cell_blocks->cell_block_dim;
         ++cell_x)
    {
      u32 cell_pos = (cell_y * cell_blocks->cell_block_dim) + cell_x;
      CellState *cell_state = cell_block->cell_states + cell_pos;
      CellState *cell_previous_state = cell_block->cell_previous_states + cell_pos;

      *cell_state = initialise_cell_state(cell_initialisation_options, position);
      *cell_previous_state = *cell_state;
    }
  }
}


CellBlock **
get_cell_block_slot(CellBlocks *cell_blocks, s32vec2 search_cell_block_position)
{
  CellBlock **result = 0;

  u32 cell_block_hash = search_cell_block_position.x * 7 + search_cell_block_position.y * 13;
  cell_block_hash %= cell_blocks->hashmap_size;

  CellBlock **hash_slot = cell_blocks->hashmap + cell_block_hash;

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
/// @param [in] cell_blocks                    Pointer to an initialised Universe.
/// @param [in] search_cell_block_position  The position of the CellBlock to get.
///
/// @returns 0 if the CellBlock already exists.
CellBlock *
create_cell_block(CellBlocks *cell_blocks, CellInitialisationOptions *cell_initialisation_options, s32vec2 search_cell_block_position)
{
  CellBlock *result = 0;

  CellBlock **cell_block_slot = get_cell_block_slot(cell_blocks, search_cell_block_position);

  if (*cell_block_slot == 0)
  {
    *cell_block_slot = allocate_cell_block(cell_blocks, search_cell_block_position);
    result = *cell_block_slot;

    init_cells(cell_blocks, cell_initialisation_options, result, search_cell_block_position);
  }
  else
  {
    // CellBlock already exists.
  }

  return result;
}


CellBlock *
create_uninitialised_cell_block(CellBlocks *cell_blocks, s32vec2 search_cell_block_position)
{
  CellBlock *result = 0;

  CellBlock **cell_block_slot = get_cell_block_slot(cell_blocks, search_cell_block_position);

  if (*cell_block_slot == 0)
  {
    *cell_block_slot = allocate_cell_block(cell_blocks, search_cell_block_position);
    result = *cell_block_slot;
  }

  return result;
}


CellBlock *
get_or_create_uninitialised_cell_block(CellBlocks *cell_blocks, s32vec2 search_cell_block_position)
{
  CellBlock *result = 0;

  CellBlock **cell_block_slot = get_cell_block_slot(cell_blocks, search_cell_block_position);

  if (*cell_block_slot == 0)
  {
    *cell_block_slot = allocate_cell_block(cell_blocks, search_cell_block_position);
  }

  result = *cell_block_slot;

  return result;
}


/// Returns the CellBlock from the hashmap, creating it if necessary.

/// Indexes the Universe hashmap to retrieve the CellBlock at search_cell_block_position.  Creates a
///   new CellBlock on the heap, if it doesn't exist, then initialise all of its Cell%s.
///
/// @param [in] cell_blocks                    Pointer to an initialised Universe.
/// @param [in] search_cell_block_position  The position of the CellBlock to get.
///
/// @returns 0 on error
CellBlock *
get_or_create_cell_block(CellBlocks *cell_blocks, CellInitialisationOptions *cell_initialisation_options, s32vec2 search_cell_block_position)
{
  CellBlock *result = 0;

  CellBlock **cell_block_slot = get_cell_block_slot(cell_blocks, search_cell_block_position);

  if (*cell_block_slot == 0)
  {
    *cell_block_slot = allocate_cell_block(cell_blocks, search_cell_block_position);
    result = *cell_block_slot;

    init_cells(cell_blocks, cell_initialisation_options, result, search_cell_block_position);
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
/// @param [in] cell_blocks                    Pointer to an initialised Universe.
/// @param [in] search_cell_block_position  The position of the CellBlock to get.
///
/// @returns 0 on error
CellBlock *
get_existing_cell_block(CellBlocks *cell_blocks, s32vec2 search_cell_block_position)
{
  CellBlock *result = 0;

  CellBlock **cell_block_slot = get_cell_block_slot(cell_blocks, search_cell_block_position);

  if (*cell_block_slot != 0)
  {
    result = *cell_block_slot;
  }

  return result;
}


void
delete_cell_block(CellBlocks *cell_blocks, s32vec2 search_cell_block_position)
{
  CellBlock **cell_block_slot = get_cell_block_slot(cell_blocks, search_cell_block_position);
  CellBlock *cell_block = *cell_block_slot;

  if (cell_block_slot != 0 &&
      cell_block != 0)
  {
    // Preserve any chain
    *cell_block_slot = cell_block->next_block;
    un_allocate(cell_block);
  }
}


/// Returns the offset of a given cell_coord into a block given the cell_blocks.
///
/// Offset can be used to index into CellBlock.previous_cell_states and CellBlock.cell_states
///
u32
get_cell_index_in_block(CellBlocks *cell_blocks, s32vec2 cell_coord)
{
  assert(cell_coord.x >= 0);
  assert(cell_coord.y >= 0);
  assert(cell_coord.x < cell_blocks->cell_block_dim);
  assert(cell_coord.y < cell_blocks->cell_block_dim);

  u32 result = (cell_coord.y * cell_blocks->cell_block_dim) + cell_coord.x;

  return result;
}
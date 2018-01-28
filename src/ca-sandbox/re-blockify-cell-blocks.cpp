#include "ca-sandbox/re-blockify-cell-blocks.h"

#include "ca-sandbox/cell-blocks.h"
#include "ca-sandbox/cell-block-coordinate-system.h"


void
convert_cell_block_position(s32vec2 block_position, s32vec2 cell_position, u32 old_cell_block_dim, u32 new_cell_block_dim, s32vec2 *new_block_position, s32vec2 *new_cell_position)
{
  // TODO:  This does not handle -ve block_positions correctly!

  s32vec2 old_block_position_cells = vec2_multiply(block_position, old_cell_block_dim);
  old_block_position_cells = vec2_add(old_block_position_cells, cell_position);
  *new_block_position = vec2_multiply(old_block_position_cells, (1.0 / new_cell_block_dim));

  s32vec2 new_block_position_cells = vec2_multiply(*new_block_position, new_cell_block_dim);
  *new_cell_position = vec2_subtract(old_block_position_cells, new_block_position_cells);
}


// Copies the CellState%s stored in cell_blocks into result, with the result's cell block dimensions
//
void
re_blockify_cell_blocks(CellBlocks *cell_blocks, CellBlocks *result)
{
  for (u32 old_cell_block_slot = 0;
       old_cell_block_slot < cell_blocks->hashmap_size;
       ++old_cell_block_slot)
  {
    CellBlock *old_cell_block = cell_blocks->hashmap[old_cell_block_slot];

    while (old_cell_block != 0)
    {
      s32vec2 cell_position;
      for (cell_position.y = 0;
           cell_position.y < cell_blocks->cell_block_dim;
           ++cell_position.y)
      {
        for (cell_position.x = 0;
             cell_position.x < cell_blocks->cell_block_dim;
             ++cell_position.x)
        {
          u32 cell_index = get_cell_index_in_block(cell_blocks, cell_position);
          CellState cell_state = old_cell_block->cell_states[cell_index];
          CellState cell_previous_state = old_cell_block->cell_previous_states[cell_index];

          s32vec2 new_block_position;
          s32vec2 new_cell_position;

          convert_cell_block_position(old_cell_block->block_position, cell_position, cell_blocks->cell_block_dim, result->cell_block_dim, &new_block_position, &new_cell_position);

          CellBlock *new_cell_block = get_or_create_uninitialised_cell_block(result, new_block_position);
          u32 new_cell_block_cell_index = get_cell_index_in_block(result, new_cell_position);

          CellState *new_cell_state = new_cell_block->cell_states + new_cell_block_cell_index;
          CellState *new_cell_previous_state = new_cell_block->cell_previous_states + new_cell_block_cell_index;

          *new_cell_state = cell_state;
          *new_cell_previous_state = cell_previous_state;
        }
      }

      // Follow any hashmap collision chains
      old_cell_block = old_cell_block->next_block;
    }
  }
}
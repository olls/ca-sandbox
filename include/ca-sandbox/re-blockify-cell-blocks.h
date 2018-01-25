#ifndef RE_BLOCKIFY_CELL_BLOCKS
#define RE_BLOCKIFY_CELL_BLOCKS

#include "ca-sandbox/cell-blocks.h"


void
convert_cell_block_position(s32vec2 block_position, s32vec2 cell_position, u32 old_cell_block_dim, u32 new_cell_block_dim, s32vec2 *new_block_position, s32vec2 *new_cell_position);


void
re_blockify_cell_blocks(CellBlocks *cell_blocks, CellBlocks *result);


#endif
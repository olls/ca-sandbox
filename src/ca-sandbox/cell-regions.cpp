#include "ca-sandbox/cell-regions.h"


CellRegion *
get_cell_region_by_name(CellRegions *cell_regions, String *search_name)
{
  CellRegion *result = 0;

  for (u32 test_index = 0;
       test_index < cell_regions->regions.n_elements;
       ++test_index)
  {
    CellRegion *test_cell_region = Array::get(cell_regions->regions, test_index);

    if (strings_equal(&test_cell_region->name, search_name))
    {
      result = test_cell_region;
      break;
    }
  }

  return result;
}


void
copy_cell_blocks(CellBlocks *from, CellBlocks *to, s32vec2 start_block, s32vec2 start_cell, s32vec2 end_block, s32vec2 end_cell, s32vec2 to_offset = {})
{
  to->cell_block_dim = from->cell_block_dim;
  const u32 cell_block_states_size = from->cell_block_dim * from->cell_block_dim * sizeof(CellState);

  for (u32 from_cell_block_slot = 0;
       from_cell_block_slot < from->hashmap_size;
       ++from_cell_block_slot)
  {
    CellBlock *from_cell_block = from->hashmap[from_cell_block_slot];

    while (from_cell_block != 0)
    {
      s32vec2 to_block_position = vec2_add(from_cell_block->block_position, to_offset);
      CellBlock *to_cell_block = get_or_create_uninitialised_cell_block(to, to_block_position);

      memcpy(to_cell_block->cell_states, from_cell_block->cell_states, cell_block_states_size);

      // Follow any hashmap collision chains
      from_cell_block = from_cell_block->next_block;
    }
  }
}


void
make_new_region(CellRegions *cell_regions, Universe *universe, const char *name, s32vec2 start_selection_block, s32vec2 start_selection_cell, s32vec2 end_selection_block, s32vec2 end_selection_cell)
{
  CellRegion new_region = {
    .name = new_string(name),
    .cell_blocks = {}
  };

  init_cell_hashmap(&new_region.cell_blocks);

  // Use same cell_block_dim as original
  new_region.cell_blocks.cell_block_dim = universe->cell_block_dim;

  s32vec2 offset = vec2_multiply(start_selection_block, -1);
  copy_cell_blocks(universe, &new_region.cell_blocks, start_selection_block, start_selection_cell, end_selection_block, end_selection_cell, offset);

  Array::add(cell_regions->regions, new_region);
}


s32vec2
get_cell_blocks_dimentions(CellBlocks *cell_blocks)
{
  s32vec2 result;

  s32vec2 lowest_coords = {};
  s32vec2 highest_coords = {};

  b32 first_block_found = false;
  for (u32 cell_block_slot = 0;
       cell_block_slot < cell_blocks->hashmap_size;
       ++cell_block_slot)
  {
    CellBlock *cell_block = cell_blocks->hashmap[cell_block_slot];

    while (cell_block != 0)
    {
      if (!first_block_found)
      {
        first_block_found = true;
        lowest_coords = cell_block->block_position;
        highest_coords = cell_block->block_position;
      }
      else
      {
        if (cell_block->block_position.x < lowest_coords.x)
        {
          lowest_coords.x = cell_block->block_position.x;
        }
        else if (cell_block->block_position.x > highest_coords.x)
        {
          highest_coords.x = cell_block->block_position.x;
        }

        if (cell_block->block_position.y < lowest_coords.y)
        {
          lowest_coords.y = cell_block->block_position.y;
        }
        else if (cell_block->block_position.y > highest_coords.y)
        {
          highest_coords.y = cell_block->block_position.y;
        }
      }

      cell_block = cell_block->next_block;
    }
  }

  result = vec2_subtract(highest_coords, lowest_coords);

  return result;
}
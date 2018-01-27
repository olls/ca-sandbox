#include "ca-sandbox/cell-regions.h"
#include "ca-sandbox/minimap.h"

#include "ca-sandbox/cell-block-coordinate-system.h"


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
      // TODO: start/end cell boundaries as well as block

      if (from_cell_block->block_position.x >= start_block.x &&
          from_cell_block->block_position.y >= start_block.y &&
          from_cell_block->block_position.x <= end_block.x &&
          from_cell_block->block_position.y <= end_block.y)
      {
        s32vec2 to_block_position = vec2_add(from_cell_block->block_position, to_offset);
        CellBlock *to_cell_block = get_or_create_uninitialised_cell_block(to, to_block_position);

        s32vec2 this_block_start_cell = {0, 0};
        if (from_cell_block->block_position.x == start_block.x)
        {
          this_block_start_cell.x = start_cell.x;
        }
        if (from_cell_block->block_position.y == start_block.y)
        {
          this_block_start_cell.y = start_cell.y;
        }
        s32vec2 this_block_end_cell = {(s32)from->cell_block_dim, (s32)from->cell_block_dim};
        if (from_cell_block->block_position.x == end_block.x)
        {
          this_block_end_cell.x = end_cell.x;
        }
        if (from_cell_block->block_position.y == end_block.y)
        {
          this_block_end_cell.y = end_cell.y;
        }

        s32vec2 cell_position;
        for (cell_position.y = this_block_start_cell.y;
             cell_position.y < this_block_end_cell.y;
             ++cell_position.y)
        {
          for (cell_position.x = this_block_start_cell.x;
               cell_position.x < this_block_end_cell.x;
               ++cell_position.x)
          {
            u32 cell_index = get_cell_index_in_block(to, cell_position);
            CellState *from_cell_state = from_cell_block->cell_states + cell_index;
            CellState *to_cell_state = to_cell_block->cell_states + cell_index;

            *to_cell_state = *from_cell_state;
          }
        }
      }

      // Follow any hashmap collision chains
      from_cell_block = from_cell_block->next_block;
    }
  }
}


void
make_region_texture(CellRegion *region, Universe *universe, GLuint minimap_framebuffer, CellDrawing *cell_drawing, CellInstancing *cell_instancing, OpenGL_Buffer *cell_vertices_buffer)
{
  region->texture_size = {200, 150};
  region->texture = create_minimap_texture(region->texture_size, minimap_framebuffer);

  if (region->texture != 0)
  {
    upload_cell_instances(&region->cell_blocks, cell_instancing);
    draw_minimap_texture(&region->cell_blocks, cell_instancing, cell_drawing, cell_vertices_buffer, minimap_framebuffer, region->texture, region->texture_size);
  }
}


void
make_new_region(CellRegions *cell_regions, CellSelectionsUI *cell_selections_ui, Universe *universe, const char *name, GLuint minimap_framebuffer, CellDrawing *cell_drawing, CellInstancing *cell_instancing, OpenGL_Buffer *cell_vertices_buffer)
{
  if (cell_selections_ui->selection_made)
  {
    char *name_memory = allocate(char, strlen(name));
    copy_string(name_memory, name, strlen(name));

    String name_string = {};
    name_string.start = name_memory;
    name_string.end = name_string.start + strlen(name);

    CellRegion new_region = {
      .name = name_string,
      .cell_blocks = {},
      .texture = 0
    };

    init_cell_hashmap(&new_region.cell_blocks);

    // Use same cell_block_dim as original
    new_region.cell_blocks.cell_block_dim = universe->cell_block_dim;

    new_region.start_block = cell_selections_ui->selection_start.cell_block_position;
    new_region.end_block = cell_selections_ui->selection_end.cell_block_position;
    new_region.start_cell = vec2_to_s32vec2(vec2_multiply(cell_selections_ui->selection_start.cell_position, universe->cell_block_dim));
    new_region.end_cell = vec2_to_s32vec2(vec2_multiply(cell_selections_ui->selection_end.cell_position, universe->cell_block_dim));

    s32vec2 offset = vec2_multiply(new_region.start_block, -1);
    copy_cell_blocks(universe, &new_region.cell_blocks, new_region.start_block, new_region.start_cell, new_region.end_block, new_region.end_cell, offset);

    make_region_texture(&new_region, universe, minimap_framebuffer, cell_drawing, cell_instancing, cell_vertices_buffer);

    CellRegion *result = Array::add(cell_regions->regions, new_region);
  }
}


void
get_cell_blocks_dimentions(CellBlocks *cell_blocks, s32vec2 *lowest_coords, s32vec2 *highest_coords)
{
  *lowest_coords = {};
  *highest_coords = {};

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
        *lowest_coords = cell_block->block_position;
        *highest_coords = cell_block->block_position;
      }
      else
      {
        if (cell_block->block_position.x < lowest_coords->x)
        {
          lowest_coords->x = cell_block->block_position.x;
        }
        else if (cell_block->block_position.x > highest_coords->x)
        {
          highest_coords->x = cell_block->block_position.x;
        }

        if (cell_block->block_position.y < lowest_coords->y)
        {
          lowest_coords->y = cell_block->block_position.y;
        }
        else if (cell_block->block_position.y > highest_coords->y)
        {
          highest_coords->y = cell_block->block_position.y;
        }
      }

      cell_block = cell_block->next_block;
    }
  }
}
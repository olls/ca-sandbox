#include "ca-sandbox/cell-regions.h"

#include "ca-sandbox/minimap.h"
#include "ca-sandbox/cell-block-coordinate-system.h"

#include <GL/glew.h>


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
copy_cell_blocks(CellBlocks *from, CellBlocks *to, s32vec2 start_block, s32vec2 start_cell, s32vec2 end_block, s32vec2 end_cell, s32vec2 to_block_offset, s32vec2 to_cell_offset)
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
      if (from_cell_block->block_position.x >= start_block.x &&
          from_cell_block->block_position.y >= start_block.y &&
          from_cell_block->block_position.x <= end_block.x &&
          from_cell_block->block_position.y <= end_block.y)
      {
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
            u32 from_cell_index = get_cell_index_in_block(to, cell_position);
            CellState *from_cell_state = from_cell_block->cell_states + from_cell_index;

            s32vec2 to_block_position = vec2_add(from_cell_block->block_position, to_block_offset);
            s32vec2 to_cell_position = vec2_add(cell_position, to_cell_offset);
            normalise_cell_coord(to, &to_block_position, &to_cell_position);
            CellBlock *to_cell_block = get_or_create_uninitialised_cell_block(to, to_block_position);

            u32 to_cell_index = get_cell_index_in_block(to, to_cell_position);
            CellState *to_cell_state = to_cell_block->cell_states + to_cell_index;

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
    Border region_border = {
      .type = BorderType::FIXED,
      .min_corner_block = region->start_block,
      .min_corner_cell = region->start_cell,
      .max_corner_block = region->end_block,
      .max_corner_cell = region->end_cell
    };
    upload_cell_instances(&region->cell_blocks, region_border, cell_instancing);
    draw_minimap_texture(&region->cell_blocks, cell_instancing, cell_drawing, cell_vertices_buffer, minimap_framebuffer, region->texture, region->texture_size);
  }
}


CellRegion
make_new_region(CellSelectionsUI *cell_selections_ui, Universe *universe, const char *name, GLuint minimap_framebuffer, CellDrawing *cell_drawing, CellInstancing *cell_instancing, OpenGL_Buffer *cell_vertices_buffer)
{
  CellRegion result = {
    .name = {},
    .cell_blocks = {},
    .texture = 0
  };

  char *name_memory = allocate(char, strlen(name));
  copy_string(name_memory, name, strlen(name));

  result.name.start = name_memory;
  result.name.end = result.name.start + strlen(name);

  init_cell_hashmap(&result.cell_blocks);

  // Use same cell_block_dim as original
  result.cell_blocks.cell_block_dim = universe->cell_block_dim;

  s32vec2 from_start_block = cell_selections_ui->selection_start.cell_block_position;
  s32vec2 from_end_block = cell_selections_ui->selection_end.cell_block_position;
  s32vec2 from_start_cell = vec2_to_s32vec2(vec2_multiply(cell_selections_ui->selection_start.cell_position, universe->cell_block_dim));
  s32vec2 from_end_cell = vec2_to_s32vec2(vec2_multiply(cell_selections_ui->selection_end.cell_position, universe->cell_block_dim));

  s32vec2 offset = vec2_multiply(cell_selections_ui->selection_start.cell_block_position, -1);

  copy_cell_blocks(universe, &result.cell_blocks, from_start_block, from_start_cell, from_end_block, from_end_cell, offset);

  result.start_block = {0, 0};
  result.end_block = vec2_add(cell_selections_ui->selection_end.cell_block_position, offset);

  result.start_cell = from_start_cell;
  result.end_cell = from_end_cell;

  make_region_texture(&result, universe, minimap_framebuffer, cell_drawing, cell_instancing, cell_vertices_buffer);

  return result;
}


void
add_region_to_universe(Universe *universe, CellRegion *copy_from, UniversePosition place_position)
{
  assert(copy_from->cell_blocks.cell_block_dim == universe->cell_block_dim);

  s32vec2 cell_dim = vec2_multiply(vec2_subtract(copy_from->end_block, copy_from->start_block), copy_from->cell_blocks.cell_block_dim);
  cell_dim = vec2_add(cell_dim, copy_from->end_cell);
  cell_dim = vec2_subtract(cell_dim, copy_from->start_cell);
  s32vec2 midpoint_offset_cell = vec2_multiply(cell_dim, 0.5);

  s32vec2 start_offset_block = copy_from->start_block;
  s32vec2 start_offset_cell = vec2_multiply(copy_from->start_cell, -1);

  s32vec2 mouse_offset_cell = vec2_to_s32vec2(vec2_multiply(place_position.cell_position, copy_from->cell_blocks.cell_block_dim));
  s32vec2 mouse_offset_block = place_position.cell_block_position;

  s32vec2 block_offset = vec2_add(start_offset_block, mouse_offset_block);
  s32vec2 cell_offset = vec2_subtract(vec2_add(start_offset_cell, mouse_offset_cell), midpoint_offset_cell);

  normalise_cell_coord(&copy_from->cell_blocks, &block_offset, &cell_offset);

  copy_cell_blocks(&copy_from->cell_blocks, universe, copy_from->start_block, copy_from->start_cell, copy_from->end_block, copy_from->end_cell, block_offset, cell_offset);
}


void
delete_region(CellRegions *cell_regions, u32 region_index)
{
  CellRegion& region = cell_regions->regions[region_index];

  glDeleteTextures(1, &region.texture);

  Array::remove(cell_regions->regions, region_index);
}
#ifndef CELL_REGIONS_H_DEF
#define CELL_REGIONS_H_DEF

#include "engine/types.h"

#include "ca-sandbox/cell-blocks.h"
#include "ca-sandbox/cell-block-coordinate-system.h"
#include "ca-sandbox/universe.h"
#include "ca-sandbox/ui/cell-selections-ui.h"
#include "ca-sandbox/cell-drawing.h"

#include "engine/my-array.h"
#include "engine/text.h"

#include <GL/glew.h>

// Have a separate CellBlock hashmap for storing regions of universe - like a clipboard.


struct CellRegion
{
  String name;
  CellBlocks cell_blocks;

  s32vec2 start_block;
  s32vec2 start_cell;
  s32vec2 end_block;
  s32vec2 end_cell;

  GLuint texture;
  s32vec2 texture_size;
};


struct CellRegions
{
  Array::Array<CellRegion> regions;

  b32 clipboard_region_in_use;
  CellRegion clipboard_region;
};


CellRegion *
get_cell_region_by_name(CellRegions *cell_regions, String search_name);


void
copy_cell_blocks(CellBlocks *from, CellBlocks *to, s32vec2 start_block, s32vec2 start_cell, s32vec2 end_block, s32vec2 end_cell, s32vec2 to_block_offset = {}, s32vec2 to_cell_offset = {});


CellRegion
make_new_region(CellSelectionsUI *cell_selections_ui, Universe *universe, const char *name, GLuint minimap_framebuffer, CellDrawing *cell_drawing, CellInstancing *cell_instancing, OpenGL_Buffer *cell_vertices_buffer);


void
add_region_to_universe(Universe *universe, CellRegion *region, UniversePosition place_position);


void
delete_region(CellRegions *cell_regions, u32 region_index);


#endif
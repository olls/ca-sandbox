#ifndef CELL_REGIONS_UI_H_DEF
#define CELL_REGIONS_UI_H_DEF

#include "ca-sandbox/cell-block-coordinate-system.h"
#include "ca-sandbox/cell-regions.h"
#include "ca-sandbox/universe.h"

#include "engine/opengl-buffer.h"


const u32 BUFFER_NAME_MAX_LENGTH = 512;


struct CellRegionsUI
{
  b32 making_selection;
  b32 making_selection_dragging;

  s32vec2 start_selection_block;
  s32vec2 start_selection_cell;

  s32vec2 end_selection_block;
  s32vec2 end_selection_cell;

  char new_region_name_buffer[BUFFER_NAME_MAX_LENGTH];
};


void
do_cell_regions_ui(CellRegionsUI *cell_regions_ui, CellRegions *cell_regions, Universe *universe, UniversePosition mouse_universe_pos, b32 *mouse_click_consumed);


u32
debug_cell_region_selection_drawing_upload(CellRegionsUI *cell_regions_ui, Universe *universe, OpenGL_Buffer *general_universe_vbo, OpenGL_Buffer *general_universe_ibo);


#endif
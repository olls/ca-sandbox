#ifndef CELL_REGIONS_UI_H_DEF
#define CELL_REGIONS_UI_H_DEF

#include "ca-sandbox/cell-block-coordinate-system.h"
#include "ca-sandbox/cell-regions.h"
#include "ca-sandbox/universe.h"
#include "ca-sandbox/cell-drawing.h"
#include "ca-sandbox/cell-selections-ui.h"

#include "engine/opengl-buffer.h"


const u32 BUFFER_NAME_MAX_LENGTH = 512;


struct CellRegionsUI
{
  char new_region_name_buffer[BUFFER_NAME_MAX_LENGTH];
};


void
do_cell_regions_ui(CellRegionsUI *cell_regions_ui, CellRegions *cell_regions, Universe *universe, CellSelectionsUI *cell_selections_ui, GLuint minimap_framebuffer, CellDrawing *cell_drawing, CellInstancing *cell_instancing, OpenGL_Buffer *cell_vertices_buffer, UniversePosition mouse_universe_pos, b32 *mouse_click_consumed);


#endif
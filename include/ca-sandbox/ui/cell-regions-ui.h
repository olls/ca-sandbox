#ifndef CELL_REGIONS_UI_H_DEF
#define CELL_REGIONS_UI_H_DEF

#include "ca-sandbox/cell-block-coordinate-system.h"
#include "ca-sandbox/cell-regions.h"
#include "ca-sandbox/universe.h"

#include "ca-sandbox/ui/cell-selections-ui.h"

const u32 BUFFER_NAME_MAX_LENGTH = 512;


struct CellRegionsUI
{
  char new_region_name_buffer[BUFFER_NAME_MAX_LENGTH];
  b32 make_new_region;

  b32 placing_region;
  u32 placing_region_index;

  b32 placing_clipboard_region;
};


void
do_cell_regions_ui(CellRegionsUI *cell_regions_ui, CellRegions *cell_regions, CellSelectionsUI *cell_selections_ui);


void
update_cell_regions(CellRegionsUI *cell_regions_ui, CellRegions *cell_regions, CellSelectionsUI *cell_selections_ui, Universe *universe,
                    GLuint minimap_framebuffer, CellDrawing *cell_drawing, CellInstancing *cell_instancing, OpenGL_Buffer *cell_vertices_buffer,
                    UniversePosition mouse_universe_position, b32 *mouse_click_consumed);

#endif
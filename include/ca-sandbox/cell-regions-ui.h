#ifndef CELL_REGIONS_UI_H_DEF
#define CELL_REGIONS_UI_H_DEF

#include "ca-sandbox/cell-block-coordinate-system.h"
#include "ca-sandbox/cell-regions.h"
#include "ca-sandbox/universe.h"
#include "ca-sandbox/cell-selections-ui.h"

const u32 BUFFER_NAME_MAX_LENGTH = 512;


struct CellRegionsUI
{
  char new_region_name_buffer[BUFFER_NAME_MAX_LENGTH];
  b32 make_new_region;

  b32 placing_region;
  u32 placing_region_index;
};


void
do_cell_regions_ui(CellRegionsUI *cell_regions_ui, CellRegions *cell_regions, Universe *universe, CellSelectionsUI *cell_selections_ui, UniversePosition mouse_universe_position, b32 *mouse_click_consumed);


#endif
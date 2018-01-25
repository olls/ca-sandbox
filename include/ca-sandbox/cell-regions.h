#ifndef CELL_REGIONS_H_DEF
#define CELL_REGIONS_H_DEF

#include "engine/types.h"

#include "ca-sandbox/cell-blocks.h"
#include "ca-sandbox/universe.h"

#include "engine/my-array.h"
#include "engine/text.h"

// Have a separate CellBlock hashmap for storing regions of universe - like a clipboard.


struct CellRegion
{
  String name;
  CellBlocks cell_blocks;
};


struct CellRegions
{
  Array::Array<CellRegion> regions;
};


CellRegion *
get_cell_region_by_name(CellRegions *cell_regions, String search_name);


void
make_new_region(CellRegions *cell_regions, Universe *universe, const char *name, s32vec2 start_selection_block, s32vec2 start_selection_cell, s32vec2 end_selection_block, s32vec2 end_selection_cell);


#endif
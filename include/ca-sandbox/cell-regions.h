#ifndef CELL_REGIONS_H_DEF
#define CELL_REGIONS_H_DEF

#include "engine/types.h"

#include "ca-sandbox/cell-blocks.h"
#include "ca-sandbox/cell-block-coordinate-system.h"
#include "ca-sandbox/universe.h"

#include "engine/my-array.h"
#include "engine/text.h"

#include <GL/glew.h>

// Have a separate CellBlock hashmap for storing regions of universe - like a clipboard.


struct CellRegion
{
  String name;
  CellBlocks cell_blocks;

  GLuint texture;
  s32vec2 texture_size;
};


struct CellRegions
{
  Array::Array<CellRegion> regions;
};


CellRegion *
get_cell_region_by_name(CellRegions *cell_regions, String search_name);


CellRegion *
make_new_region(CellRegions *cell_regions, Universe *universe, const char *name, UniversePosition selection_start, UniversePosition selection_end);


void
get_cell_blocks_dimentions(CellBlocks *cell_blocks, s32vec2 *lowest_coords, s32vec2 *highest_coords);


#endif
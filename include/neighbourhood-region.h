#ifndef NEIGHBOURHOOD_REGION_H_DEF
#define NEIGHBOURHOOD_REGION_H_DEF

#include "types.h"
#include "vectors.h"


enum struct NeighbourhoodRegionShape
{
  VON_NEUMANN,
  MOORE,
  ONE_DIM
};


u32
get_neighbourhood_region_n_cells(NeighbourhoodRegionShape shape, u32 size);


s32vec2
get_neighbourhood_region_cell_delta(NeighbourhoodRegionShape shape, u32 size, u32 index);


s32vec2
get_neighbourhood_region_coverage(NeighbourhoodRegionShape shape, u32 size);


u32
get_neighbourhood_region_centre_index(NeighbourhoodRegionShape shape, u32 size);


#endif

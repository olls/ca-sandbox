#include "neighbourhood-region.h"

#include "types.h"
#include "assert.h"
#include "print.h"
#include "maths.h"
#include "vectors.h"

/// @file
/// @brief  Utility functions for indexing the neighbours of cell according to different
///           neighbourhood region types and sizes.
///


/// Returns the total number of cells within the neighbourhood region, including the centre cell
///
u32
get_neighbourhood_region_n_cells(NeighbourhoodRegionShape shape, u32 size)
{
  u32 result = 0;

  switch (shape)
  {
    case (NeighbourhoodRegionShape::VON_NEUMANN):
    {
      // 1 => 5
      //   #
      // # # #
      //   #
      //
      // 2 => 9
      //     #
      //     #
      // # # # # #
      //     #
      //     #
      result = (size * 4) + 1;
    } break;

    case (NeighbourhoodRegionShape::MOORE):
    {
      // 1 => 9
      // # # #
      // # # #
      // # # #
      //
      // 2 => 25
      // # # # # #
      // # # # # #
      // # # # # #
      // # # # # #
      // # # # # #
      result = pow(((size*2) + 1), 2);
    } break;

    case (NeighbourhoodRegionShape::ONE_DIM):
    {
      // 1 => 3
      // # # #
      //
      // 2 => 5
      // # # # # #
      result = (size * 2) + 1;
    } break;
  }

  return result;
}


/// Returns the size of the square bound containing the entire neighbourhood region.
///
s32vec2
get_neighbourhood_region_coverage(NeighbourhoodRegionShape shape, u32 size)
{
  s32vec2 result;

  switch (shape)
  {
    case (NeighbourhoodRegionShape::VON_NEUMANN):
    case (NeighbourhoodRegionShape::MOORE):
    {
      result.x = 2*size + 1;
      result.y = 2*size + 1;
    } break;

    case (NeighbourhoodRegionShape:: ONE_DIM):
    {
      result.x = size + 1;
      result.y = 1;
    } break;
  }

  return result;
}



/// Convert neighbour index into direction, indices are ordered from left-to-right, top-to-bottom.
///   Used to loop over all neighbours in execute_transision_function().
///
s32vec2
get_neighbourhood_region_cell_delta(NeighbourhoodRegionShape shape, u32 size, u32 index)
{
  assert(index < get_neighbourhood_region_n_cells(shape, size));

  s32vec2 result;
  switch (shape)
  {
    case (NeighbourhoodRegionShape::MOORE):
    {
      u32 side_length = ((size * 2) + 1);

      result = (s32vec2){(s32)(index % side_length),
                         (s32)(index / side_length)};

      result = vec2_subtract(result, size);

    } break;
    case (NeighbourhoodRegionShape::VON_NEUMANN):
    {
      u32 distance_from_centre;

      if (index < size)
      {
        // Up spoke
        result = (s32vec2){ 0, -1};
        distance_from_centre = size - index;
      }
      else if (index < 2 * size)
      {
        // Left spoke
        result = (s32vec2){-1,  0};
        distance_from_centre = 2 * size - index;
      }
      else if (index == size * 2)
      {
        // Centre
        result = (s32vec2){0, 0};
        distance_from_centre = 0;
      }
      else if (index < (size * 3) + 1)
      {
        // Right spoke
        result = (s32vec2){ 1,  0};
        distance_from_centre = index - (size * 2);
      }
      else if (index < (size * 4) + 1)
      {
        // Bottom spoke
        result = (s32vec2){ 0,  1};
        distance_from_centre = index - (size * 3);
      }
      else
      {
        print("Error: index passed into get_von_neumann_neighbour_delta is too large for size.\n");
        assert(0);
      }

      result = vec2_multiply(result, distance_from_centre);

    } break;

    case (NeighbourhoodRegionShape::ONE_DIM):
    {
      result = (s32vec2){(s32)(index - size), 0};

    } break;
  }

  return result;
}


/// Returns the neighbour index for the central cell
u32
get_neighbourhood_region_centre_index(NeighbourhoodRegionShape shape, u32 size)
{
  u32 result = get_neighbourhood_region_n_cells(shape, size) / 2;
  return result;
}
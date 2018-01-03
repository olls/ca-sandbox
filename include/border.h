#ifndef BORDER_H_DEF
#define BORDER_H_DEF

#include "types.h"
#include "vectors.h"
#include "universe.h"


enum struct BorderType
{
  FIXED,
  INFINITE,
  TORUS
};


/// Border types:
/// - FIXED: A fixed border from border_min_corner_{block,cell} to border_max_corner_{block,cell}
/// - INFINITE: Automata expands infinitely
/// - TORUS: Wraps around from top to bottom and side to side, at the borders.
struct Border
{
  BorderType type;
  s32vec2 min_corner_block;
  s32vec2 min_corner_cell;
  s32vec2 max_corner_block;
  s32vec2 max_corner_cell;
};


b32
check_border(Border border, s32vec2 cell_block_position, s32vec2 cell_position);


bool
get_neighbouring_cell_state(Border *border, Universe *universe, s32vec2 neighbouring_cell_delta, s32vec2 subject_block, s32vec2 subject_cell, CellState *resulting_state);


#endif
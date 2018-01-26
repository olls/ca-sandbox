#ifndef CELL_SELECTIONS_UI_H_DEF
#define CELL_SELECTIONS_UI_H_DEF

#include "ca-sandbox/cell-block-coordinate-system.h"
#include "ca-sandbox/cell-drawing.h"

#include "engine/types.h"
#include "engine/opengl-buffer.h"


struct CellSelectionsUI
{
  b32 making_selection;

  b32 selection_made;
  UniversePosition selection_start;
  UniversePosition selection_end;
};


void
do_cell_selections_ui(CellSelectionsUI *cell_selections_ui, UniversePosition mouse_universe_pos, b32 *mouse_click_consumed);


u32
cell_selections_drawing_upload(CellSelectionsUI *cell_selections_ui, Universe *universe, OpenGL_Buffer *general_universe_vbo, OpenGL_Buffer *general_universe_ibo);


u32
debug_cell_selections_drawing_upload(CellSelectionsUI *cell_selections_ui, Universe *universe, OpenGL_Buffer *general_universe_vbo, OpenGL_Buffer *general_universe_ibo);


#endif
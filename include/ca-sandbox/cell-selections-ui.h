#ifndef CELL_SELECTIONS_UI_H_DEF
#define CELL_SELECTIONS_UI_H_DEF

#include "ca-sandbox/cell-block-coordinate-system.h"
#include "ca-sandbox/cell-drawing.h"

#include "engine/types.h"
#include "engine/opengl-buffer.h"
#include "engine/drawing.h"


struct CellSelectionsUI
{
  b32 making_selection;

  b32 selection_made;
  UniversePosition selection_start;
  UniversePosition selection_end;
};


void
do_cell_selections_ui(CellSelectionsUI *cell_selections_ui, UniversePosition mouse_universe_pos, b32 *mouse_click_consumed);


void
cell_selections_drawing_upload(CellSelectionsUI *cell_selections_ui, Universe *universe, mat4x4 universe_projection_matrix, mat4x4 aspect_ratio, OpenGL_Buffer *vertex_buffer, OpenGL_Buffer *colour_buffer, OpenGL_Buffer *ibo, BufferDrawingLocation *vertices_position, BufferDrawingLocation *colours_position, BufferDrawingLocation *ibo_position);


u32
debug_cell_selections_drawing_upload(CellSelectionsUI *cell_selections_ui, Universe *universe, OpenGL_Buffer *general_universe_vbo, OpenGL_Buffer *general_universe_ibo);


#endif
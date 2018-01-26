#ifndef CELLS_EDITOR_H_DEF
#define CELLS_EDITOR_H_DEF

#include "engine/vectors.h"

#include "ca-sandbox/cell-blocks.h"
#include "ca-sandbox/cell-block-coordinate-system.h"
#include "ca-sandbox/named-states.h"


struct CellsEditor
{
  b32 cell_highlighted;  //< Indicates whether a cell is currently being hovered over with the mouse
  UniversePosition highlighted_cell;  //< The position of the cell currently highlighted
  CellState highlighted_cell_state;

  b32 cell_block_highlighted;  //< Indicates whether a potential cell block is currently being hovered over with the mouse
  s32vec2 highlighted_cell_block;  //< The position of the potential cell block currently highlighted

  CellState active_state;

  CellState drag_state;
  b32 currently_dragging_state;
  b32 currently_dragging_cell_block_creation;

  /// The position of the cell block the mouse was hovering over, when the context menu was opened.
  s32vec2 current_context_menu_cell_block;
  /// The name state of the cell which the mouse was hovering over, when the context menu was opened.
  String current_contex_menu_cell_state;
};


void
do_cells_editor(CellsEditor *cells_editor, Universe *universe, CellInitialisationOptions *cell_initialisation_options, NamedStates *named_states, UniversePosition mouse_universe_position, b32 *mouse_click_consumed);


#endif
#ifndef CELLS_EDITOR_H_DEF
#define CELLS_EDITOR_H_DEF

#include "vectors.h"
#include "view-panning.h"

#include "universe.h"
#include "cell-block-coordinate-system.h"
#include "named-states.h"


struct CellsEditor
{
  /// Indicates whether a cell is currently being hovered over with the mouse
  b32 cell_highlighted;

  /// The position of the cell currently highlighted
  UniversePosition highlighted_cell;

  /// Needed for de-bouncing clicks
  u64 last_click_time;
};


void
do_cells_editor(CellsEditor *cells_editor, Universe *universe, NamedStates *named_states, UniversePosition mouse_universe_position, b32 panning_last_frame, u64 frame_time);


#endif
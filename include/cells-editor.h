#ifndef CELLS_EDITOR_H_DEF
#define CELLS_EDITOR_H_DEF

#include "vectors.h"
#include "view-panning.h"

#include "universe.h"
#include "cell-block-coordinate-system.h"


struct CellsEditor
{
  /// Indicates whether a cell is currently being hovered over with the mouse
  b32 cell_highlighted;

  /// The position of the cell currently highlighted
  UniversePosition highlighted_cell;
};


void
do_cells_editor(CellsEditor *cells_editor, Universe *universe, UniversePosition mouse_universe_position);


#endif
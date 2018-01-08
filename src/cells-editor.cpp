#include "cells-editor.h"

#include "vectors.h"
#include "view-panning.h"

#include "universe.h"

#include "imgui.h"


void
do_cells_editor(CellsEditor *cells_editor, Universe *universe, UniversePosition universe_mouse_position)
{
  if (ImGui::Begin("Cells Editor"))
  {
    CellBlock *hovered_cell_block = get_existing_cell_block(universe, universe_mouse_position.cell_block_position);

    if (hovered_cell_block == 0)
    {
      cells_editor->cell_highlighted = false;
    }
    else
    {
      ImGui::Text("hovered_cell_block: %d %d", hovered_cell_block->block_position.x, hovered_cell_block->block_position.y);

      // TODO: Check we are hovering a cell as well as block

      cells_editor->cell_highlighted = true;
      cells_editor->highlighted_cell.cell_block_position = universe_mouse_position.cell_block_position;

      s32vec2 cell_position = vec2_to_s32vec2(vec2_multiply(universe_mouse_position.cell_position, universe->cell_block_dim));
      vec2 cell_position_proportion = vec2_divide(s32vec2_to_vec2(cell_position), universe->cell_block_dim);

      cells_editor->highlighted_cell.cell_position = cell_position_proportion;
    }

    if (cells_editor->cell_highlighted)
    {
      ImGui::Text("Highlighted Cell: {%d %d} {%f %f}", cells_editor->highlighted_cell.cell_block_position.x, cells_editor->highlighted_cell.cell_block_position.y,
                                                       cells_editor->highlighted_cell.cell_position.x, cells_editor->highlighted_cell.cell_position.y);
    }
  }

  ImGui::End();
}
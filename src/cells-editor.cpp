#include "cells-editor.h"

#include "vectors.h"
#include "view-panning.h"
#include "print.h"

#include "universe.h"
#include "named-states.h"

#include "imgui.h"

const u32 CLICK_DEBOUNCE = 100000; // 1/10th second


void
do_cells_editor(CellsEditor *cells_editor, Universe *universe, CellInitialisationOptions *cell_initialisation_options, NamedStates *named_states, UniversePosition universe_mouse_position, b32 currently_panning)
{
  cells_editor->cell_highlighted = false;
  cells_editor->cell_block_highlighted = false;

  ImGuiIO& io = ImGui::GetIO();
  if (!(io.MousePos.x == -1 &&
        io.MousePos.y == -1) &&
      !io.WantCaptureMouse)
  {
    CellBlock *hovered_cell_block = get_existing_cell_block(universe, universe_mouse_position.cell_block_position);

    if (hovered_cell_block == 0)
    {
      // CellBlock editing

      // Show cell block outline
      cells_editor->cell_block_highlighted = true;
      cells_editor->highlighted_cell_block = universe_mouse_position.cell_block_position;

      if (!currently_panning &&
          (ImGui::IsMouseClicked(0) ||
           (ImGui::IsMouseDragging() &&
            cells_editor->currently_dragging_cell_block_creation)))
      {
        cells_editor->currently_dragging_cell_block_creation = true;
        create_cell_block(universe, cell_initialisation_options, universe_mouse_position.cell_block_position);
      }
    }
    else
    {
      // Cell editing

      // TODO: Check we are hovering a cell as well as block (i.e: account for gaps between cells)

      cells_editor->cell_highlighted = true;
      cells_editor->highlighted_cell.cell_block_position = universe_mouse_position.cell_block_position;

      s32vec2 cell_position = vec2_to_s32vec2(vec2_multiply(universe_mouse_position.cell_position, universe->cell_block_dim));
      vec2 cell_position_proportion = vec2_divide(s32vec2_to_vec2(cell_position), universe->cell_block_dim);

      cells_editor->highlighted_cell.cell_position = cell_position_proportion;

      if (!currently_panning &&
          !cells_editor->currently_dragging_cell_block_creation &&
          ImGui::IsMouseClicked(0))
      {
        cells_editor->currently_dragging_state = true;

        Cell *cell = get_cell_from_block(universe, hovered_cell_block, cell_position);

        // Choose which state we are dragging

        if (cell->state != cells_editor->active_state)
        {
          cells_editor->drag_state = cells_editor->active_state;
        }
        else
        {
          cells_editor->drag_state = 0;
        }
        cell->state = cells_editor->drag_state;
      }
      else if (!currently_panning &&
               cells_editor->currently_dragging_state &&
               ImGui::IsMouseDragging())
      {
        cells_editor->currently_dragging_state = true;

        Cell *cell = get_cell_from_block(universe, hovered_cell_block, cell_position);
        cell->state = cells_editor->drag_state;
      }
    }
  }

  if (!ImGui::IsMouseDown(0))
  {
    cells_editor->currently_dragging_cell_block_creation = false;
    cells_editor->currently_dragging_state = false;
  }
}
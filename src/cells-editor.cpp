#include "cells-editor.h"

#include "vectors.h"
#include "view-panning.h"

#include "universe.h"
#include "named-states.h"

#include "imgui.h"

const u32 CLICK_DEBOUNCE = 100000; // 1/10th second


void
do_cells_editor(CellsEditor *cells_editor, Universe *universe, NamedStates *named_states, UniversePosition universe_mouse_position, b32 panning_last_frame, u64 frame_time)
{
  cells_editor->cell_highlighted = false;

  ImGuiIO& io = ImGui::GetIO();
  if (!(io.MousePos.x == -1 &&
        io.MousePos.y == -1) &&
      !io.WantCaptureMouse)
  {
    CellBlock *hovered_cell_block = get_existing_cell_block(universe, universe_mouse_position.cell_block_position);

    if (hovered_cell_block != 0)
    {
      // TODO: Check we are hovering a cell as well as block (i.e: account for gaps between cells)

      cells_editor->cell_highlighted = true;
      cells_editor->highlighted_cell.cell_block_position = universe_mouse_position.cell_block_position;

      s32vec2 cell_position = vec2_to_s32vec2(vec2_multiply(universe_mouse_position.cell_position, universe->cell_block_dim));
      vec2 cell_position_proportion = vec2_divide(s32vec2_to_vec2(cell_position), universe->cell_block_dim);

      cells_editor->highlighted_cell.cell_position = cell_position_proportion;

      if (io.MouseReleased[0] && !panning_last_frame && frame_time >= cells_editor->last_click_time + CLICK_DEBOUNCE)
      {
        cells_editor->last_click_time = frame_time;
        Cell *cell = get_cell_from_block(universe, hovered_cell_block, cell_position);
        cell->state = advance_state(named_states, cell->state);
      }
    }
  }
}
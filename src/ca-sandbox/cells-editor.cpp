#include "ca-sandbox/cells-editor.h"

#include "engine/vectors.h"
#include "engine/print.h"

#include "ca-sandbox/cell-blocks.h"
#include "ca-sandbox/named-states.h"
#include "ca-sandbox/view-panning.h"

#include "imgui/imgui.h"


void
do_cells_editor(CellsEditor *cells_editor, Universe *universe, CellInitialisationOptions *cell_initialisation_options, NamedStates *named_states, UniversePosition universe_mouse_position, b32 *mouse_click_consumed)
{
  ImGui::PushID("cells editor");

  cells_editor->cell_highlighted = false;

  ImGuiIO& io = ImGui::GetIO();
  if (!*mouse_click_consumed)
  {
    CellBlock *hovered_cell_block = get_existing_cell_block(universe, universe_mouse_position.cell_block_position);

    // TODO: Check we are hovering a cell as well as block (i.e: account for gaps between cells)

    cells_editor->cell_highlighted = true;
    cells_editor->highlighted_cell.cell_block_position = universe_mouse_position.cell_block_position;

    s32vec2 cell_position = vec2_to_s32vec2(vec2_multiply(universe_mouse_position.cell_position, universe->cell_block_dim));
    vec2 cell_position_proportion = vec2_divide(s32vec2_to_vec2(cell_position), universe->cell_block_dim);

    cells_editor->highlighted_cell.cell_position = cell_position_proportion;
    if (hovered_cell_block != 0)
    {
      u32 cell_index = get_cell_index_in_block(universe, cell_position);
      CellState hovered_cell_state = hovered_cell_block->cell_states[cell_index];

      cells_editor->highlighted_cell_state = hovered_cell_state;
    }
    else
    {
      cells_editor->highlighted_cell_state = 0;
    }

    b32 set_cell_state = false;

    if (ImGui::IsMouseClicked(0))
    {
      *mouse_click_consumed = true;
      cells_editor->currently_dragging_state = true;

      // Choose which state we are dragging

      if (hovered_cell_block == 0)
      {
        cells_editor->drag_state = cells_editor->active_state;
      }
      else
      {
        u32 cell_index = get_cell_index_in_block(universe, cell_position);
        CellState hovered_cell_state = hovered_cell_block->cell_states[cell_index];

        if (hovered_cell_state != cells_editor->active_state)
        {
          cells_editor->drag_state = cells_editor->active_state;
        }
        else
        {
          cells_editor->drag_state = 0;
        }
      }

      set_cell_state = true;
    }
    else if (cells_editor->currently_dragging_state &&
             ImGui::IsMouseDragging())
    {
      *mouse_click_consumed = true;
      cells_editor->currently_dragging_state = true;

      set_cell_state = true;
    }
    else if (ImGui::IsMouseClicked(1) && hovered_cell_block != 0)
    {
      *mouse_click_consumed = true;

      cells_editor->current_context_menu_cell_block = universe_mouse_position.cell_block_position;

      u32 cell_index = get_cell_index_in_block(universe, cell_position);
      CellState *cell_state = hovered_cell_block->cell_states + cell_index;
      cells_editor->current_contex_menu_cell_state = get_state_name(named_states, *cell_state);

      ImGui::OpenPopup("cell block context menu");
    }

    if (set_cell_state)
    {
      if (hovered_cell_block == 0)
      {
        hovered_cell_block = create_cell_block(universe, cell_initialisation_options, universe_mouse_position.cell_block_position);
      }

      u32 cell_index = get_cell_index_in_block(universe, cell_position);
      CellState *cell_state = hovered_cell_block->cell_states + cell_index;
      *cell_state = cells_editor->drag_state;
    }
  }

  if (!ImGui::IsMouseDown(0))
  {
    cells_editor->currently_dragging_state = false;
  }

  if (ImGui::BeginPopup("cell block context menu"))
  {
    ImGui::Text("Cell State: %.*s", string_length(cells_editor->current_contex_menu_cell_state), cells_editor->current_contex_menu_cell_state.start);
    ImGui::Text("Cell Block: %d %d", cells_editor->current_context_menu_cell_block.x, cells_editor->current_context_menu_cell_block.y);
    if (ImGui::Button("Delete cell block"))
    {
      ImGui::CloseCurrentPopup();
      delete_cell_block(universe, cells_editor->current_context_menu_cell_block);
    }
    ImGui::EndPopup();
  }

  ImGui::PopID();
}
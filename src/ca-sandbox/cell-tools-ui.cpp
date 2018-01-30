#include "ca-sandbox/cell-tools-ui.h"

#include "ca-sandbox/cell-tools.h"
#include "ca-sandbox/named-states.h"

#include "engine/colour.h"

#include "imgui/imgui.h"


b32
named_states_drop_down(NamedStates *named_states, CellState *selected_state)
{
  bool value_changed = false;

  String selected_state_name = get_state_name(named_states, *selected_state);

  if (ImGui::BeginCombo("###named states", selected_state_name.start))
  {
    for (u32 named_state_index = 0;
         named_state_index < named_states->states.n_elements;
         ++named_state_index)
    {
      ImGui::PushID(named_state_index);

      NamedState& named_state = named_states->states[named_state_index];
      const bool named_state_selected = (named_state.value == *selected_state);

      vec4 state_colour = get_state_colour(named_state.value);
      ImGui::PushStyleColor(ImGuiCol_Text, state_colour);

      if (ImGui::Selectable(named_state.name.start, named_state_selected))
      {
        *selected_state = named_state.value;
        value_changed = true;
      }

      ImGui::PopStyleColor();
      ImGui::PopID();
    }

    ImGui::EndCombo();
  }

  return value_changed;
}


void
do_cell_tools_ui(CellTools *cell_tools, NamedStates *named_states)
{
  b32 set_selection_to_state = ImGui::Button("Set selection to");
  ImGui::SameLine();
  ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
  named_states_drop_down(named_states, &cell_tools->state);

  if (set_selection_to_state)
  {
    cell_tools->flags |= CellToolFlags__SetSelectionNull;
  }

  if (ImGui::Button("Delete null cell blocks"))
  {
    cell_tools->flags |= CellToolFlags__DeleteNullBlocks;
  }
}
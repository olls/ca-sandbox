#include "rule-ui.h"

#include "types.h"
#include "vectors.h"
#include "allocate.h"
#include "print.h"
#include "maths.h"
#include "util.h"
#include "file-picker.h"
#include "comparison-operator.h"
#include "extendable-array.h"

#include "neighbourhood-region.h"
#include "named-states.h"
#include "rule.h"
#include "load-rule.h"

#include <stdio.h>
#include "imgui.h"

/// @file
/// @brief  Provides GUI elements to modify the currently loaded Rule
///


b32
styled_cell_state_button(const char *id, String label, vec4 colour = {-1})
{
  ImGui::PushID(id);
  if (colour.x != -1)
  {
    ImGui::PushStyleColor(ImGuiCol_Button, colour);
  }

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {9, 6});

  b32 result = ImGui::Button(label.start, label.end);

  ImGui::PopStyleVar();
  if (colour.x != -1)
  {
    ImGui::PopStyleColor();
  }
  ImGui::PopID();

  return result;
}


void
cell_state_button(const char *id, CellState *state, NamedStates *named_states)
{
  String state_string = get_state_name(named_states, *state);
  if (styled_cell_state_button(id, state_string, get_state_colour(*state)))
  {
    *state = advance_state(named_states, *state);
  }
}


void
cell_state_button(const char *id, PatternCellState *pattern_cell, NamedStates *named_states)
{
  // TODO: This basically needs to be rewritten to handle the grouped states...

  if (pattern_cell->type == PatternCellStateType::STATE ||
      pattern_cell->type == PatternCellStateType::NOT_STATE ||
      pattern_cell->type == PatternCellStateType::OR_STATE)
  {
    cell_state_button(id, &pattern_cell->states[0], named_states);
  }
  else
  {
    String state_string = {};
    switch (pattern_cell->type)
    {
      case (PatternCellStateType::WILDCARD):
      {
        state_string = new_string("*");
      } break;
      default:
      {
        state_string = {};
      }
    }
    styled_cell_state_button(id, state_string);
  }

  if (ImGui::BeginPopupContextItem("pattern cell select wildcard context menu"))
  {
    ImGui::RadioButton("Wildcard", (s32 *)&pattern_cell->type, (s32)PatternCellStateType::WILDCARD); ImGui::SameLine();
    ImGui::RadioButton("Match State", (s32 *)&pattern_cell->type, (s32)PatternCellStateType::STATE);
    ImGui::EndPopup();
  }
}


void
display_rule_pattern(RuleConfiguration *rule_config, RulePattern *rule_pattern)
{
  ImGui::NewLine();

  // Extract all cells in rule pattern into a grid so we can print them in the correct positions

  s32vec2 neighbourhood_region_area = get_neighbourhood_region_coverage(rule_config->neighbourhood_region_shape, rule_config->neighbourhood_region_size);
  PatternCellState **cell_spacial_array = allocate(PatternCellState *, neighbourhood_region_area.x * neighbourhood_region_area.y);
  memset(cell_spacial_array, 0, sizeof(PatternCellState *) * neighbourhood_region_area.x * neighbourhood_region_area.y);

  // The position of the central cell in the spacial array
  s32vec2 centre_cell_position = vec2_divide(neighbourhood_region_area, 2);

  u32 n_cells = get_neighbourhood_region_n_cells(rule_config->neighbourhood_region_shape, rule_config->neighbourhood_region_size);
  for (u32 cell_n = 0;
       cell_n < n_cells;
       ++cell_n)
  {
    s32vec2 cell_delta = get_neighbourhood_region_cell_delta(rule_config->neighbourhood_region_shape, rule_config->neighbourhood_region_size, cell_n);

    // Add cell delta to the central position in the spacial array
    s32vec2 cell_position = vec2_add(centre_cell_position, cell_delta);

    cell_spacial_array[(cell_position.y * neighbourhood_region_area.x) + cell_position.x] = &rule_pattern->cell_states[cell_n];
  }

  // Iterate through the cell_spacial_array outputting each cell as a button
  const ImGuiStyle& style = ImGui::GetStyle();
  r32 padding = style.FramePadding.y;
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 0});

  s32vec2 position;
  for (position.x = 0;
       position.x < neighbourhood_region_area.x;
       ++position.x)
  {
    ImGui::BeginGroup();

    for (position.y = 0;
         position.y < neighbourhood_region_area.y;
         ++position.y)
    {
      u32 cell_spacial_position = (position.y * neighbourhood_region_area.x) + position.x;
      PatternCellState *pattern_cell = cell_spacial_array[cell_spacial_position];
      ImGui::PushID(cell_spacial_position);

      if (pattern_cell == NULL)
      {
        r32 item_vertical_spacing = ImGui::GetItemsLineHeightWithSpacing();
        ImGui::Dummy(vec2{0, item_vertical_spacing});
      }
      else
      {
        cell_state_button("cell state", pattern_cell, &rule_config->named_states);
      }
      ImGui::PopID();
    }

    ImGui::EndGroup();
    ImGui::SameLine();
  }

  ImGui::PopStyleVar();

  // Same line needs to be called here to add a gap, now we have popped the 0 spacing style.
  ImGui::SameLine();

  un_allocate(cell_spacial_array);

  // Second column
  ImGui::BeginGroup();
  {
    ImGui::AlignFirstTextHeightToWidgets();
    ImGui::Text("Result:");
    ImGui::SameLine();
    cell_state_button("resulting cell", &rule_pattern->result, &rule_config->named_states);

    ImGui::Checkbox("Count matching enabled for this pattern", (bool *)&rule_pattern->count_matching.enabled);

    if (rule_pattern->count_matching.enabled)
    {
      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {9, 6});

      ImGui::AlignFirstTextHeightToWidgets();
      ImGui::Text("n");
      ImGui::SameLine();
      for (u32 count_matching_state_n = 0;
           count_matching_state_n < rule_pattern->count_matching.group_states_used;
           ++count_matching_state_n)
      {
        cell_state_button("count matching state", &rule_pattern->count_matching.states[count_matching_state_n], &rule_config->named_states);
      }
      ImGui::SameLine();
      ImGui::Text("wildcard cells");
      ImGui::SameLine();

      // Drop down to select comparison operator
      ImGui::PushItemWidth(50);
      ImGui::Combo("##comparison operator", (s32*)&rule_pattern->count_matching.comparison, COMPARISON_OPERATOR_STRINGS, array_count(COMPARISON_OPERATOR_STRINGS));
      ImGui::PopItemWidth();

      ImGui::SameLine();

      // Int input to select count matching n
      s32 new_count_matching_n = (s32)rule_pattern->count_matching.comparison_n;
      ImGui::PushItemWidth(85);
      ImGui::InputInt("##count matching n", &new_count_matching_n);
      ImGui::PopItemWidth();

      ImGui::PopStyleVar();

      rule_pattern->count_matching.comparison_n = (u32)clamp(0, (s32)n_cells, new_count_matching_n);
    }
  }
  ImGui::EndGroup();

  ImGui::NewLine();
}


void
do_rule_ui(RuleUI *rule_ui, Rule *rule, RuleCreationThread *rule_creation_thread)
{
  ImGuiWindowFlags window_flags = 0;

  if (ImGui::Begin("Rule Editor", NULL, window_flags))
  {
    u32 string_length = strlen(rule_ui->file_picker.selected_file);
    ImGui::Text("Rule file: %.*s", string_length, rule_ui->file_picker.selected_file);

    if (ImGui::Button("Change rule file"))
    {
      rule_ui->file_picker.active = true;
      rule_ui->file_picker.current_item = 0;
      rule_ui->file_picker.root_directory = ".";
      copy_string(rule_ui->file_picker.current_path, "rules", 6);
    }

    if (rule_ui->file_picker.active)
    {
      file_picker("Rule file picker", &rule_ui->file_picker);
    }

    ImGui::SameLine();
    if (ImGui::Button("Reload rule file"))
    {
      rule_ui->reload_rule_file = true;
    }

    if (rule_creation_thread->currently_running)
    {
      ImGui::ProgressBar(((r64)rule_creation_thread->progress.done / (r64)rule_creation_thread->progress.total));
    }
    else
    {
      if (ImGui::Button("Build rule tree from patterns"))
      {
        start_build_rule_tree_thread(rule_creation_thread, rule);
      }
    }

    // Display all rule patterns
    ImGui::TextWrapped("Modify each of the patterns below by clicking on the state buttons to change the state which it matches, or right clicking on them to change them to a wildcard match.");

    for (u32 rule_n = 0;
         rule_n < rule->config.rule_patterns.n_elements;
         ++rule_n)
    {
      RulePattern *rule_pattern = rule->config.rule_patterns.get(rule_n);

      ImGui::PushID(rule_n);
      char label[32]; sprintf(label, "Pattern %d", rule_n);
      if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen))
      {
        display_rule_pattern(&rule->config, rule_pattern);
      }
      ImGui::PopID();
    }

    if (ImGui::Button("Add pattern"))
    {
      RulePattern *new_rule_pattern = rule->config.rule_patterns.get_new_element();
    }
  }

  ImGui::End();
}
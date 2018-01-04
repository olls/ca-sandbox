#include "rule-ui.h"

#include "types.h"
#include "vectors.h"
#include "allocate.h"
#include "imgui.h"
#include "print.h"
#include "file-picker.h"

#include "neighbourhood-region.h"
#include "named-states.h"
#include "rule.h"
#include "load-rule.h"


CellState
advance_state(NamedStates *named_states, CellState previous)
{
  CellState result;

  if (previous == named_states->n_states - 1)
  {
    result = 0;
  }
  else
  {
    result = previous + 1;
  }

  return result;
}


void
display_rule_pattern(RuleConfiguration *rule_config, RulePattern *rule_pattern)
{
  u32 n_cells = get_neighbourhood_region_n_cells(rule_config->neighbourhood_region_shape, rule_config->neighbourhood_region_size);

  s32vec2 neighbourhood_region_area = get_neighbourhood_region_coverage(rule_config->neighbourhood_region_shape, rule_config->neighbourhood_region_size);

  // Extract all cells in rule pattern into a grid so we can print them in the correct positions

  PatternCellState **cell_spacial_array = allocate(PatternCellState *, neighbourhood_region_area.x * neighbourhood_region_area.y);
  memset(cell_spacial_array, 0, sizeof(PatternCellState *) * neighbourhood_region_area.x * neighbourhood_region_area.y);

  // The position of the central cell in the spacial array
  s32vec2 centre_cell_position = vec2_divide(neighbourhood_region_area, 2);

  for (u32 cell_n = 0;
       cell_n < n_cells;
       ++cell_n)
  {
    s32vec2 cell_delta = get_neighbourhood_region_cell_delta(rule_config->neighbourhood_region_shape, rule_config->neighbourhood_region_size, cell_n);

    // Add cell delta to the central position in the spacial array
    s32vec2 cell_position = vec2_add(centre_cell_position, cell_delta);

    cell_spacial_array[(cell_position.y * neighbourhood_region_area.x) + cell_position.x] = &rule_pattern->cell_states[cell_n];
  }

  ImGui::PushID(rule_pattern);
  ImGui::BeginGroup();

  // Iterate through the cell_spacial_array outputting each cell as a button

  const ImGuiStyle& style = ImGui::GetStyle();
  r32 padding = style.FramePadding.x;

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

      if (pattern_cell == NULL)
      {
        r32 item_vertical_spacing = ImGui::GetItemsLineHeightWithSpacing() - padding;
        ImGui::Dummy(vec2{0, item_vertical_spacing});
      }
      else
      {
        String state_string = {};

        if (pattern_cell->type == PatternCellStateType::STATE)
        {
          state_string = rule_config->named_states.state_names[pattern_cell->state];
        }
        else if (pattern_cell->type == PatternCellStateType::WILDCARD)
        {
          state_string = new_string("*");
        }
        else if (pattern_cell->type == PatternCellStateType::NOT_USED)
        {
          state_string = new_string("-");
        }

        // Push cell_spacial_position to give this button a unique id from the other cells in this pattern
        ImGui::PushID(cell_spacial_position);
        if (ImGui::Button(state_string.start, state_string.end))
        {
          pattern_cell->state = advance_state(&rule_config->named_states, pattern_cell->state);
        }
        ImGui::PopID();
      }
    }

    ImGui::EndGroup();
    ImGui::SameLine();
  }

  un_allocate(cell_spacial_array);

  ImGui::Text("Result:");

  String result_state_string = rule_config->named_states.state_names[rule_pattern->result];
  ImGui::SameLine();
  ImGui::PushID("resulting cell");

  if (ImGui::Button(result_state_string.start, result_state_string.end))
  {
    rule_pattern->result = advance_state(&rule_config->named_states, rule_pattern->result);
  }

  ImGui::PopID();

  ImGui::EndGroup();
  ImGui::PopID();

  ImGui::NewLine();
}


void
do_rule_ui(RuleUI *rule_ui, Rule *rule)
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
      rule_ui->file_picker.current_path[0] = '\0';
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

    // Display all rule patterns

    for (u32 rule_n = 0;
         rule_n < rule->config.rule_patterns.n_elements;
         ++rule_n)
    {
      RulePattern *rule_pattern = (RulePattern *)get_from_extendible_array(&rule->config.rule_patterns, rule_n);
      display_rule_pattern(&rule->config, rule_pattern);
    }
  }

  if (ImGui::Button("Apply rules"))
  {
    build_rule_tree(rule);
  }

  ImGui::End();
}
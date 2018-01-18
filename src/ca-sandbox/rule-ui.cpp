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
#include "colour.h"

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

  b32 result = ImGui::Button(label.start, label.start + string_length(label));

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


b32
cell_state_button(String label, vec2 item_spacing, vec2 frame_padding)
{
  b32 result;

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, item_spacing);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, frame_padding);

  result = ImGui::Button(label.start, label.end);

  ImGui::PopStyleVar(2);
  return result;
}


/// Creates a list of check-boxes for all the named states, allowing the user to choose which states
///   are in the PatternCell.states[] group.
///
void
multi_cell_state_selector(PatternCellState *pattern_cell, NamedStates *named_states)
{
  ImGui::BeginChild("multi cell state selector", {0, 200});

  for (u32 state_n = 0;
       state_n < named_states->states.n_elements;
       ++state_n)
  {
    NamedState *named_state = named_states->states.get(state_n);

    bool state_was_selected = false;
    u32 previously_selected_state_position_in_group;

    // Determine whether the state is currently in the pattern cell's group
    for (u32 state_n = 0;
         state_n < pattern_cell->group_states_used;
         ++state_n)
    {
      CellState test_state = pattern_cell->states[state_n];
      if (test_state == named_state->value)
      {
        state_was_selected = true;
        previously_selected_state_position_in_group = state_n;
        break;
      }
    }

    ImGui::PushID(state_n);
    vec4 state_colour = get_state_colour(named_state->value);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, state_colour);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, lighten_colour(state_colour));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, darken_colour(state_colour));
    ImGui::PushStyleColor(ImGuiCol_CheckMark, {0, 0, 0, 0.5});

    bool state_now_selected = state_was_selected;
    b32 pressed = ImGui::Checkbox("###make state active", &state_now_selected);

    ImGui::PopStyleColor(4);
    ImGui::SameLine();

    ImGui::PushItemWidth(-70);
    ImGui::Text("%.*s", string_length(named_state->name), named_state->name.start);

    ImGui::PopID();

    if (pressed &&
        !state_was_selected &&
        state_now_selected)
    {
      // Put state in states[] if the check-box is checked and state wasn't already in the group.

      if (pattern_cell->group_states_used == MAX_PATTERN_STATES_GROUP)
      {
        // TODO: Make this an ImGui error message.
        assert(!"Ran out of state slots in PatternCellState group.");
      }
      else
      {
        pattern_cell->states[pattern_cell->group_states_used] = named_state->value;
        pattern_cell->group_states_used += 1;
      }
    }
    else if (pressed &&
             state_was_selected &&
             !state_now_selected)
    {
      // Remove state from states[] if the check-box is not checked but state __was__ in the group.

      if (pattern_cell->group_states_used > 1)
      {
        pattern_cell->group_states_used -= 1;

        // Move last state in the group into the position of the state we are removing from the group.

        u32 move_to_position = previously_selected_state_position_in_group;
        u32 move_from_position = pattern_cell->group_states_used;

        // If the positions are equal, the state we are removing must have been at the end of the
        //   group, so decrementing group_states_used is enough to remove it.
        if (move_to_position != move_from_position)
        {
          pattern_cell->states[move_to_position] = pattern_cell->states[move_from_position];
        }
      }
    }
  }

  ImGui::EndChild();
}


void
pattern_cell_state_menu(PatternCellState *pattern_cell, NamedStates *named_states)
{
  ImGui::Combo("Cell Pattern Type", (s32*)&pattern_cell->type, PATTERN_CELL_STATE_TYPE_NAMES, array_count(PATTERN_CELL_STATE_TYPE_NAMES));

  switch (pattern_cell->type)
  {
    case (PatternCellStateType::WILDCARD):
    {
      ImGui::Spacing();
      ImGui::TextWrapped("This cell will match any state.  It will also contribute towards a \"count matching\" constraint, if it matches the count matching state.");
    } break;

    case (PatternCellStateType::STATE):
    {
      ImGui::TextWrapped("This cell will match if any of the following selected states match.");
      ImGui::Spacing();
      multi_cell_state_selector(pattern_cell, named_states);

    } break;

    case (PatternCellStateType::NOT_STATE):
    {
      ImGui::TextWrapped("This cell will match any cell, as long as it isn't one of the following states.");
      ImGui::Spacing();
      multi_cell_state_selector(pattern_cell, named_states);

    } break;

    case (PatternCellStateType::OR_STATE):
    {
      ImGui::TextWrapped("This pattern will match, if one or more of the \"OR\" cell patterns match.");
      ImGui::Spacing();
      multi_cell_state_selector(pattern_cell, named_states);

     } break;
  }
}


/// Create GUI for modifying a single PatternCellState.
///
/// Draws a button showing the basic pattern cell configuration, with a menu when clicked for
/// modification.
///
void
pattern_cell_state_button(PatternCellState *pattern_cell, NamedStates *named_states, vec2 item_spacing, vec2 frame_padding)
{
  String button_label = {};

  switch (pattern_cell->type)
  {
    case (PatternCellStateType::WILDCARD):
    {
      button_label = new_string("*");
    } break;

    case (PatternCellStateType::STATE):
    {
      if (pattern_cell->group_states_used == 1)
      {
        CellState state = pattern_cell->states[0];
        String state_name = get_state_name(named_states, state);
        button_label = state_name;
      }
      else
      {
        button_label = new_string_fmt("[%d states]", pattern_cell->group_states_used);
      }
    } break;

    case (PatternCellStateType::NOT_STATE):
    {
      CellState state = pattern_cell->states[0];
      String state_name = get_state_name(named_states, state);
      button_label = new_string_fmt("!%.*s", string_length(state_name), state_name.start);
    } break;

    case (PatternCellStateType::OR_STATE):
    {
      CellState state = pattern_cell->states[0];
      String state_name = get_state_name(named_states, state);
      button_label = new_string_fmt("(%.*s)", string_length(state_name), state_name.start);
    } break;
  }

  b32 button_pressed = cell_state_button(button_label, item_spacing, frame_padding);

  if (button_pressed)
  {
    ImGui::OpenPopup("pattern cell state menu");
  }

  ImGui::SetNextWindowContentWidth(300);
  if (ImGui::BeginPopup("pattern cell state menu"))
  {
    pattern_cell_state_menu(pattern_cell, named_states);
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

  vec2 cell_btn_item_spacing = {1, 1};
  vec2 cell_btn_frame_padding = {9, 6};

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
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, cell_btn_frame_padding);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, cell_btn_item_spacing);
        r32 item_vertical_spacing = ImGui::GetItemsLineHeightWithSpacing() - cell_btn_item_spacing.y;
        ImGui::Dummy(vec2{0, item_vertical_spacing});
        ImGui::PopStyleVar(2);
      }
      else
      {
        pattern_cell_state_button(pattern_cell, &rule_config->named_states, cell_btn_item_spacing, cell_btn_frame_padding);
      }

      ImGui::PopID();
    }

    ImGui::EndGroup();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, cell_btn_item_spacing);
    ImGui::SameLine();
    ImGui::PopStyleVar(1);
  }

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
    u32 filename_length = strlen(rule_ui->file_picker.selected_file);
    ImGui::Text("Rule file: %.*s", filename_length, rule_ui->file_picker.selected_file);

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

    if (ImGui::Button("Save rule file"))
    {
      rule_ui->save_rule_file = true;
    }

    if (!rule_creation_thread->currently_running)
    {
      if (ImGui::Button("Build rule tree from patterns"))
      {
        start_build_rule_tree_thread(rule_creation_thread, rule);
      }
    }
    else
    {
      ImGui::ProgressBar(((r64)rule_creation_thread->progress.done / (r64)rule_creation_thread->progress.total));
    }

    // Display all rule patterns
    ImGui::TextWrapped(
"""Modify each of the patterns below by clicking on the state buttons to \
change the state which it matches, or right clicking on them to change them \
to a wildcard match.""");

    for (u32 rule_pattern_n = 0;
         rule_pattern_n < rule->config.rule_patterns.n_elements;
         ++rule_pattern_n)
    {
      RulePattern *rule_pattern = rule->config.rule_patterns.get(rule_pattern_n);

      char label[64 + MAX_COMMENT_LENGTH];
      sprintf(label, "Pattern %d: %s###pattern_%d", rule_pattern_n, rule_pattern->comment, rule_pattern_n);

      ImGui::PushID(rule_pattern_n);
      if (ImGui::CollapsingHeader(label))
      {
        ImGui::InputText("Comment", rule_pattern->comment, MAX_COMMENT_LENGTH);
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
#include "ca-sandbox/ui/rule-ui.h"

#include "engine/types.h"
#include "engine/vectors.h"
#include "engine/allocate.h"
#include "engine/print.h"
#include "engine/maths.h"
#include "engine/util.h"
#include "engine/comparison-operator.h"
#include "engine/my-array.h"
#include "engine/colour.h"
#include "engine/human-time.h"

#include "ca-sandbox/neighbourhood-region.h"
#include "ca-sandbox/named-states.h"
#include "ca-sandbox/rule.h"
#include "ca-sandbox/load-rule.h"

#include "interface/file-picker.h"

#include "imgui/imgui.h"

#include <stdio.h>

/// @file
/// @brief  Provides GUI elements to modify the currently loaded Rule
///


/// Creates a list of check-boxes for all the named states, allowing the user to choose which states
///   are in the states[] group.
///
void
multi_cell_state_selector(CellStateGroup *states_group, NamedStates *named_states)
{
  ImGui::BeginChild("multi cell state selector", {0, 200});

  for (u32 state_n = 0;
       state_n < named_states->states.n_elements;
       ++state_n)
  {
    NamedState& named_state = named_states->states[state_n];

    bool state_was_selected = false;
    u32 previously_selected_state_position_in_group;

    // Determine whether the state is currently in the pattern cell's group
    for (u32 state_n = 0;
         state_n < states_group->states_used;
         ++state_n)
    {
      CellState test_state = states_group->states[state_n];
      if (test_state == named_state.value)
      {
        state_was_selected = true;
        previously_selected_state_position_in_group = state_n;
        break;
      }
    }

    ImGui::PushID(state_n);
    vec4 state_colour = get_state_colour(named_state.value);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, state_colour);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, lighten_colour(state_colour));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, darken_colour(state_colour));
    ImGui::PushStyleColor(ImGuiCol_CheckMark, {0, 0, 0, 0.5});

    bool state_now_selected = state_was_selected;
    b32 pressed = ImGui::Checkbox("###make state active", &state_now_selected);

    ImGui::PopStyleColor(4);
    ImGui::SameLine();

    ImGui::PushItemWidth(-70);
    ImGui::Text("%.*s", string_length(named_state.name), named_state.name.start);

    ImGui::PopID();

    if (pressed &&
        !state_was_selected &&
        state_now_selected)
    {
      // Put state in states[] if the check-box is checked and state wasn't already in the group.

      if (states_group->states_used == MAX_PATTERN_STATES_GROUP)
      {
        // TODO: Make this an ImGui error message.
        assert(!"Ran out of state slots in PatternCellState group.");
      }
      else
      {
        states_group->states[states_group->states_used] = named_state.value;
        states_group->states_used += 1;
      }
    }
    else if (pressed &&
             state_was_selected &&
             !state_now_selected)
    {
      // Remove state from states[] if the check-box is not checked but state __was__ in the group.

      if (states_group->states_used > 1)
      {
        states_group->states_used -= 1;

        // Move last state in the group into the position of the state we are removing from the group.

        u32 move_to_position = previously_selected_state_position_in_group;
        u32 move_from_position = states_group->states_used;

        // If the positions are equal, the state we are removing must have been at the end of the
        //   group, so decrementing group_states_used is enough to remove it.
        if (move_to_position != move_from_position)
        {
          states_group->states[move_to_position] = states_group->states[move_from_position];
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
      multi_cell_state_selector(&pattern_cell->states_group, named_states);

    } break;

    case (PatternCellStateType::NOT_STATE):
    {
      ImGui::TextWrapped("This cell will match any cell, as long as it isn't one of the following states.");
      ImGui::Spacing();
      multi_cell_state_selector(&pattern_cell->states_group, named_states);

    } break;

    case (PatternCellStateType::OR_STATE):
    {
      ImGui::TextWrapped("This pattern will match, if one or more of the \"OR\" cell patterns match.");
      ImGui::Spacing();
      multi_cell_state_selector(&pattern_cell->states_group, named_states);

     } break;
  }
}


/// Creates a cell state button with the correct size, (TODO: and colour?)
///
b32
cell_state_button(String label, vec2 item_spacing, vec2 frame_padding)
{
  b32 result;

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, item_spacing);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, frame_padding);

  result = ImGui::Button(label.start);

  ImGui::PopStyleVar(2);
  return result;
}


/// Create GUI for modifying a single CellState
///
/// Takes an ID, as a convenience because this function is called "one-off" for the result state so
///   it saves us wrapping the call in a PushID/PodID.
///
void
cell_state_button(const char *id, CellState *state, NamedStates *named_states, vec2 item_spacing, vec2 frame_padding)
{
  String state_name = get_state_name(named_states, *state);

  ImGui::PushID(id);
  b32 button_pressed = cell_state_button(state_name, item_spacing, frame_padding);
  ImGui::PopID();

  if (button_pressed)
  {
    ImGui::OpenPopup("cell state menu");
  }

  ImGui::SetNextWindowContentSize({300, 0});
  if (ImGui::BeginPopup("cell state menu"))
  {
    // Displays radio button selector in pop-up-menu to pick CellState

    ImGui::BeginChild("cell state selector", {0, 200});

    for (u32 state_n = 0;
         state_n < named_states->states.n_elements;
         ++state_n)
    {
      NamedState& named_state = named_states->states[state_n];

      ImGui::PushID(state_n);
      vec4 state_colour = get_state_colour(named_state.value);
      ImGui::PushStyleColor(ImGuiCol_FrameBg, state_colour);
      ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, lighten_colour(state_colour));
      ImGui::PushStyleColor(ImGuiCol_FrameBgActive, darken_colour(state_colour));
      ImGui::PushStyleColor(ImGuiCol_CheckMark, {0, 0, 0, 0.5});

      b32 pressed = ImGui::RadioButton("###state name", state, named_state.value);

      ImGui::PopStyleColor(4);
      ImGui::SameLine();

      ImGui::PushItemWidth(-70);
      ImGui::Text("%.*s", string_length(named_state.name), named_state.name.start);

      ImGui::PopID();
    }

    ImGui::EndChild();

    ImGui::EndPopup();
  }
}


/// Builds a label for a button representing a CellStateGroup
///
String
cell_state_group_button_label(CellStateGroup *cell_state_group, NamedStates *named_states)
{
  String result = {};

  if (cell_state_group->states_used == 1)
  {
    CellState state = cell_state_group->states[0];
    String state_name = get_state_name(named_states, state);
    result = state_name;
  }
  else
  {
    result = new_string_fmt("[%d states]", cell_state_group->states_used);
  }

  return result;
}


/// Create GUI for modifying a CellStateGroup.
///
/// This is currently only used for the count_matching states, but it replicates the behaviour of
///   the pattern_cell_state_button, if it was restricted to a STATE type.
///
void
cell_state_group_button(CellStateGroup *cell_state_group, NamedStates *named_states, vec2 item_spacing, vec2 frame_padding)
{
  String button_label = cell_state_group_button_label(cell_state_group, named_states);
  b32 button_pressed = cell_state_button(button_label, item_spacing, frame_padding);

  if (button_pressed)
  {
    ImGui::OpenPopup("cell state group menu");
  }

  ImGui::SetNextWindowContentSize({300, 0});
  if (ImGui::BeginPopup("cell state group menu"))
  {
    multi_cell_state_selector(cell_state_group, named_states);
    ImGui::EndPopup();
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
      button_label = cell_state_group_button_label(&pattern_cell->states_group, named_states);
    } break;

    case (PatternCellStateType::NOT_STATE):
    {
      String group_string = cell_state_group_button_label(&pattern_cell->states_group, named_states);
      button_label = new_string_fmt("!%.*s", string_length(group_string), group_string.start);
    } break;

    case (PatternCellStateType::OR_STATE):
    {
      String group_string = cell_state_group_button_label(&pattern_cell->states_group, named_states);
      button_label = new_string_fmt("(%.*s)", string_length(group_string), group_string.start);
    } break;

    default: assert(false);
  }

  b32 button_pressed = cell_state_button(button_label, item_spacing, frame_padding);

  if (button_pressed)
  {
    ImGui::OpenPopup("pattern cell state menu");
  }

  ImGui::SetNextWindowContentSize({300, 0});
  if (ImGui::BeginPopup("pattern cell state menu"))
  {
    pattern_cell_state_menu(pattern_cell, named_states);
    ImGui::EndPopup();
  }
}


/// Creates the GUI for displaying a single RulePattern
///
void
display_rule_pattern(RuleConfiguration *rule_config, RulePattern *rule_pattern)
{
  ImGui::Spacing();

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
    PatternCellState *pattern_cell_state = rule_pattern->cell_states + cell_n;

    // Add cell delta to the central position in the spacial array to offset from 0,0 (top-left)
    s32vec2 cell_delta = get_neighbourhood_region_cell_delta(rule_config->neighbourhood_region_shape, rule_config->neighbourhood_region_size, cell_n);
    s32vec2 cell_position = vec2_add(centre_cell_position, cell_delta);

    cell_spacial_array[(cell_position.y * neighbourhood_region_area.x) + cell_position.x] = pattern_cell_state;
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

      if (pattern_cell == 0)
      {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, cell_btn_frame_padding);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, cell_btn_item_spacing);
        r32 item_vertical_spacing = ImGui::GetFrameHeightWithSpacing() - cell_btn_item_spacing.y;
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

  un_allocate(cell_spacial_array);

  ImGui::NewLine();
  ImGui::Spacing();

  // Result and count matching configuration
  ImGui::BeginGroup();

  ImGui::AlignTextToFramePadding();
  ImGui::Text("Result:");
  ImGui::SameLine();
  cell_state_button("resulting cell", &rule_pattern->result, &rule_config->named_states, cell_btn_item_spacing, cell_btn_frame_padding);

  ImGui::Checkbox("Count matching enabled for this pattern", (bool *)&rule_pattern->count_matching.enabled);

  if (rule_pattern->count_matching.enabled)
  {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {9, 6});

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Number of wildcard cells matching");
    ImGui::SameLine();
    ImGui::PopStyleVar();

    cell_state_group_button(&rule_pattern->count_matching.states_group, &rule_config->named_states, cell_btn_item_spacing, cell_btn_frame_padding);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {9, 6});
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

  ImGui::EndGroup();

  ImGui::NewLine();
}


/// Display the Rule UI window; contains the rule file selector, tree building, rule patterns editor
///
void
do_rule_ui(RuleUI *rule_ui, Rule *rule, RuleCreationThread *rule_creation_thread, FilesLoadedState *files_loaded_state)
{
  ImGui::Text("Rule file: %.*s", rule_ui->file_picker.selected_file.n_elements, rule_ui->file_picker.selected_file.elements);

  const char *rule_file_picker_name = "Rule file picker";
  if (ImGui::Button("Change rule file"))
  {
    ImGui::OpenPopup(rule_file_picker_name);
    rule_ui->file_picker.current_item = 0;

    Array::clear(rule_ui->file_picker.root_directory);
    append_string(rule_ui->file_picker.root_directory, new_string("."));

    Array::clear(rule_ui->file_picker.current_path);
    append_string(rule_ui->file_picker.current_path, new_string("rules"));
  }

  if (file_picker(rule_file_picker_name, &rule_ui->file_picker))
  {
    flag_load_rule_file(files_loaded_state);

  }

  if (rule_ui->file_picker.selected_file.n_elements != 0)
  {
    ImGui::SameLine();
    if (ImGui::Button("Save rule file"))
    {
      rule_ui->save_rule_file = true;
    }
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

  if (rule_creation_thread->last_build_total_time != 0)
  {
    const char *unit;
    r32 last_build_time = human_time(rule_creation_thread->last_build_total_time, &unit);
    ImGui::SameLine();
    ImGui::TextWrapped("Build took %.2f %s", last_build_time, unit);
    ImGui::Spacing();
  }

  // Display all rule patterns
  ImGui::TextWrapped(
"""Modify each of the patterns below by clicking on the state buttons to \
change the state which it matches, or right clicking on them to change them \
to a wildcard match.""");

  ImGui::BeginChild("Rule patterns");

  for (u32 rule_pattern_n = 0;
       rule_pattern_n < rule->config.rule_patterns.n_elements;
       ++rule_pattern_n)
  {
    RulePattern *rule_pattern = Array::get(rule->config.rule_patterns, rule_pattern_n);

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
    RulePattern *new_rule_pattern = Array::add(rule->config.rule_patterns);
  }

  ImGui::EndChild();
}
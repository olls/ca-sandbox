#include "ca-sandbox/save-rule-config.h"

#include "engine/print.h"
#include "engine/text.h"
#include "engine/maths.h"
#include "engine/allocate.h"
#include "engine/my-array.h"

#include "ca-sandbox/named-states.h"

#include <stdio.h>


void
serialise_neighbourhood_region(FILE *file_stream, RuleConfiguration *rule_config)
{
  const char *neighbourhood_region_shape_string;
  switch(rule_config->neighbourhood_region_shape)
  {
    case (NeighbourhoodRegionShape::VON_NEUMANN):
    {
      neighbourhood_region_shape_string = "VON_NEUMANN";
    } break;
    case (NeighbourhoodRegionShape::MOORE):
    {
      neighbourhood_region_shape_string = "MOORE";
    } break;
    case (NeighbourhoodRegionShape::ONE_DIM):
    {
      neighbourhood_region_shape_string = "ONE_DIM";
    } break;
  }
  fprintf(file_stream, "neighbourhood_region_shape: %s\n", neighbourhood_region_shape_string);
  fprintf(file_stream, "neighbourhood_region_size: %u\n", rule_config->neighbourhood_region_size);
  fprintf(file_stream, "\n");
}


void
serialise_named_states(FILE *file_stream, RuleConfiguration *rule_config)
{
  NamedStates *named_states = &rule_config->named_states;

  fprintf(file_stream, "n_states: %u\n", named_states->states.n_elements);

  for (u32 named_state_n = 0;
       named_state_n < named_states->states.n_elements;
       ++named_state_n)
  {
    NamedState& named_state = named_states->states[named_state_n];
    fprintf(file_stream, "State: %.*s\n", string_length(named_state.name), named_state.name.start);
  }
  fprintf(file_stream, "\n");
}


void
serialise_null_states(FILE *file_stream, RuleConfiguration *rule_config)
{
  Array::Array<CellState> &null_states = rule_config->null_states;

  fprintf(file_stream, "null_states: ");
  for (u32 null_state_n = 0;
       null_state_n < null_states.n_elements;
       ++null_state_n)
  {
    CellState null_state = null_states[null_state_n];
    String null_state_name = get_state_name(&rule_config->named_states, null_state);

    fprintf(file_stream, "%.*s", string_length(null_state_name), null_state_name.start);

    if (null_state_n != null_states.n_elements-1)
    {
      fprintf(file_stream, ", ");
    }
  }
  fprintf(file_stream, "\n\n");
}


void
serialise_state_group(Array::Array<char>& string, CellStateGroup *group, NamedStates *named_states)
{
  if (group->states_used > 1)
  {
    append_string(string, new_string("["));
  }
  for (u32 state_n = 0;
       state_n < group->states_used;
       ++state_n)
  {
    CellState state = group->states[state_n];
    String state_name = get_state_name(named_states, state);
    append_string(string, state_name);

    if (state_n != group->states_used-1)
    {
      append_string(string, new_string(", "));
    }
  }
  if (group->states_used > 1)
  {
    append_string(string, new_string("]"));
  }
}


void
serialise_count_matching(FILE *file_stream, CountMatching *count_matching, NamedStates *named_states)
{
  if (count_matching->enabled)
  {
    Array::Array<char> states_group_string = {};
    serialise_state_group(states_group_string, &count_matching->states_group, named_states);

    const char *comparison_string = COMPARISON_OPERATOR_STRINGS[(u32)count_matching->comparison];

    fprintf(file_stream, "count_matching: %.*s, %s %d\n", states_group_string.n_elements, states_group_string.elements, comparison_string, count_matching->comparison_n);
    Array::free_array(states_group_string);
  }
}


void
serialise_pattern_cell_state(Array::Array<char>& cell_string, PatternCellState *pattern_cell, NamedStates *named_states)
{
  switch (pattern_cell->type)
  {
    case (PatternCellStateType::WILDCARD):
    {
      append_string(cell_string, new_string("*"));
    } break;
    case (PatternCellStateType::STATE):
    {
      serialise_state_group(cell_string, &pattern_cell->states_group, named_states);
    } break;
    case (PatternCellStateType::NOT_STATE):
    {
      append_string(cell_string, new_string("!"));
      serialise_state_group(cell_string, &pattern_cell->states_group, named_states);
    } break;
    case (PatternCellStateType::OR_STATE):
    {
      append_string(cell_string, new_string("("));
      serialise_state_group(cell_string, &pattern_cell->states_group, named_states);
      append_string(cell_string, new_string(")"));
    } break;
  }
}


void
serialise_rule_pattern(FILE *file_stream, RulePattern& rule_pattern, RuleConfiguration *rule_config)
{
  fprintf(file_stream, "Pattern: %s\n", rule_pattern.comment);

  String result_string = get_state_name(&rule_config->named_states, rule_pattern.result);
  fprintf(file_stream, "Result: %.*s\n", string_length(result_string), result_string.start);

  // First serialise each CellPatternState into a grid so we can print them in the correct positions

  s32vec2 neighbourhood_region_area = get_neighbourhood_region_coverage(rule_config->neighbourhood_region_shape, rule_config->neighbourhood_region_size);
  u32 n_cells_in_spacial_array = neighbourhood_region_area.x * neighbourhood_region_area.y;

  // Array of strings representing each cell
  Array::Array<char> *cell_strings = allocate(Array::Array<char>, n_cells_in_spacial_array);
  memset(cell_strings, 0, sizeof(Array::Array<char>) * n_cells_in_spacial_array);

  // Need to find the length of the widest cell for each column in this pattern so we can line up
  //   the columns correctly
  u32 max_state_lengths[neighbourhood_region_area.x];
  memset(max_state_lengths, 0, sizeof(u32) * neighbourhood_region_area.x);

  // Need to find the last column containing anything (for each row), so we don't leave trailing
  //   whitespace
  u32 last_column_used[neighbourhood_region_area.y];
  memset(last_column_used, 0, sizeof(u32) * neighbourhood_region_area.y);

  // The position of the central cell in the spacial array
  s32vec2 centre_cell_position = vec2_divide(neighbourhood_region_area, 2);

  u32 n_cells = get_neighbourhood_region_n_cells(rule_config->neighbourhood_region_shape, rule_config->neighbourhood_region_size);
  for (u32 cell_n = 0;
       cell_n < n_cells;
       ++cell_n)
  {
    PatternCellState *pattern_cell = rule_pattern.cell_states + cell_n;
    s32vec2 cell_delta = get_neighbourhood_region_cell_delta(rule_config->neighbourhood_region_shape, rule_config->neighbourhood_region_size, cell_n);

    // Add cell delta to the central position in the spacial array
    s32vec2 cell_position = vec2_add(centre_cell_position, cell_delta);
    u32 spacial_array_position = (cell_position.y * neighbourhood_region_area.x) + cell_position.x;
    Array::Array<char>& cell_string = cell_strings[spacial_array_position];

    if (pattern_cell != NULL)
    {
      serialise_pattern_cell_state(cell_string, pattern_cell, &rule_config->named_states);
    }

    u32 cell_string_length = cell_string.n_elements;
    max_state_lengths[cell_position.x] = max(max_state_lengths[cell_position.x], cell_string_length);

    last_column_used[cell_position.y] = max(last_column_used[cell_position.y], (u32)cell_position.x);
  }

  // Iterate through the cell_spacial_array outputting each cell text
  for (u32 row = 0;
       row < neighbourhood_region_area.y;
       ++row)
  {
    for (u32 column = 0;
         column < neighbourhood_region_area.x;
         ++column)
    {
      u32 cell_spacial_position = (row * neighbourhood_region_area.x) + column;
      Array::Array<char>& cell_string = cell_strings[cell_spacial_position];

      u32 padding = 0;
      if (column < last_column_used[row])
      {
        padding = max_state_lengths[column] + 2;
      }

      fprintf(file_stream, "%-*.*s", padding, cell_string.n_elements, cell_string.elements);

      Array::free_array(cell_string);
    }

    fprintf(file_stream, "\n");
  }

  serialise_count_matching(file_stream, &rule_pattern.count_matching, &rule_config->named_states);

  fprintf(file_stream, "\n\n");
}


void
serialise_rule_patterns(FILE *file_stream, RuleConfiguration *rule_config)
{
  RulePatterns& rule_patterns = rule_config->rule_patterns;

  for (u32 rule_pattern_n = 0;
       rule_pattern_n < rule_patterns.n_elements;
       ++rule_pattern_n)
  {
    RulePattern& rule_pattern = rule_patterns[rule_pattern_n];
    serialise_rule_pattern(file_stream, rule_pattern, rule_config);
  }
}


b32
save_rule_config_to_file(const char *filename, RuleConfiguration *rule_config)
{
  b32 success = true;

  FILE *file_stream = fopen(filename, "w");
  if (file_stream == NULL)
  {
    print("Error: Failed to open file %s", filename);
    success &= false;
  }
  else
  {
    serialise_neighbourhood_region(file_stream, rule_config);
    serialise_named_states(file_stream, rule_config);
    serialise_null_states(file_stream, rule_config);
    fprintf(file_stream, "\n");
    serialise_rule_patterns(file_stream, rule_config);
  }

  fclose(file_stream);

  return success;
}
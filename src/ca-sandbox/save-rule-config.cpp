#include "save-rule-config.h"

#include "print.h"
#include "text.h"
#include "maths.h"

#include "named-states.h"

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
    NamedState *named_state = named_states->states.get(named_state_n);
    fprintf(file_stream, "State: %.*s\n", string_length(named_state->name), named_state->name.start);
  }
  fprintf(file_stream, "\n");
}


void
serialise_null_states(FILE *file_stream, RuleConfiguration *rule_config)
{
  ExtendableArray<CellState> *null_states = &rule_config->null_states;

  fprintf(file_stream, "null_states: ");
  for (u32 null_state_n = 0;
       null_state_n < null_states->n_elements;
       ++null_state_n)
  {
    CellState null_state = *null_states->get(null_state_n);
    String null_state_name = get_state_name(&rule_config->named_states, null_state);

    fprintf(file_stream, "%.*s", string_length(null_state_name), null_state_name.start);

    if (null_state_n != null_states->n_elements-1)
    {
      fprintf(file_stream, ", ");
    }
  }
  fprintf(file_stream, "\n\n");
}


void
serialise_rule_pattern(FILE *file_stream, RulePattern *rule_pattern, RuleConfiguration *rule_config)
{
  fprintf(file_stream, "Pattern: %s\n", rule_pattern->comment);

  String result_string = get_state_name(&rule_config->named_states, rule_pattern->result);
  fprintf(file_stream, "Result: %.*s\n", string_length(result_string), result_string.start);

  // Extract all cells in rule pattern into a grid so we can print them in the correct positions

  s32vec2 neighbourhood_region_area = get_neighbourhood_region_coverage(rule_config->neighbourhood_region_shape, rule_config->neighbourhood_region_size);
  u32 n_cells_in_spacial_array = neighbourhood_region_area.x * neighbourhood_region_area.y;
  PatternCellState *cell_spacial_array[n_cells_in_spacial_array];
  memset(cell_spacial_array, 0, sizeof(PatternCellState *) * n_cells_in_spacial_array);

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

  // Need to find the length of the widest cell for each column in this pattern so we can line up
  //   the columns correctly.
  u32 max_state_lengths[neighbourhood_region_area.x];
  for (u32 column = 0;
       column < neighbourhood_region_area.x;
       ++column)
  {
    max_state_lengths[column] = 0;
    for (u32 row = 0;
         row < neighbourhood_region_area.y;
         ++row)
    {
      u32 this_cell_length = 0;

      u32 cell_spacial_position = (row * neighbourhood_region_area.x) + column;
      PatternCellState *pattern_cell = cell_spacial_array[cell_spacial_position];
      if (pattern_cell != 0)
      {
        switch (pattern_cell->type)
        {
          case (PatternCellStateType::WILDCARD):
          {
            this_cell_length = 1;
          } break;
          case (PatternCellStateType::STATE):
          {
            if (pattern_cell->group_states_used > 1)
            {
              this_cell_length += 2;  // "[" ... "]"
              this_cell_length += 2 * (pattern_cell->group_states_used-1);  // ", " between each state
            }
            for (u32 group_state_n = 0;
                 group_state_n < pattern_cell->group_states_used;
                 ++group_state_n)
            {
              CellState state = pattern_cell->states[group_state_n];
              String state_name = get_state_name(&rule_config->named_states, state);
              this_cell_length += string_length(state_name);
            }
          } break;
          case (PatternCellStateType::NOT_STATE):
          {
            this_cell_length += 1;  // "!"
            CellState state = pattern_cell->states[0];
            String state_name = get_state_name(&rule_config->named_states, state);
            this_cell_length += string_length(state_name);
          } break;
          case (PatternCellStateType::OR_STATE):
          {
            this_cell_length += 2;  // "(" ... ")"
            CellState state = pattern_cell->states[0];
            String state_name = get_state_name(&rule_config->named_states, state);
            this_cell_length += string_length(state_name);
          } break;
        }
      }

      max_state_lengths[column] = max(max_state_lengths[column], this_cell_length);
    }
  }

  // Iterate through the cell_spacial_array outputting each cell as a button
  for (u32 row = 0;
       row < neighbourhood_region_area.y;
       ++row)
  {
    for (u32 column = 0;
         column < neighbourhood_region_area.x;
         ++column)
    {
      u32 max_state_length = max_state_lengths[column];

      u32 cell_spacial_position = (row * neighbourhood_region_area.x) + column;
      PatternCellState *pattern_cell = cell_spacial_array[cell_spacial_position];

      char buffer[max_state_length];
      WriteString cell_string = {
        .start = buffer,
        .current_position = buffer,
        .end = buffer + max_state_length
      };

      if (pattern_cell != NULL)
      {
        switch (pattern_cell->type)
        {
          case (PatternCellStateType::WILDCARD):
          {
            append_string(&cell_string, new_string("*"));
          } break;
          case (PatternCellStateType::STATE):
          {
            if (pattern_cell->group_states_used > 1)
            {
              append_string(&cell_string, new_string("["));
            }

            for (u32 group_state_n = 0;
                 group_state_n < pattern_cell->group_states_used;
                 ++group_state_n)
            {
              CellState state = pattern_cell->states[group_state_n];
              String state_name = get_state_name(&rule_config->named_states, state);
              append_string(&cell_string, state_name);

              if (group_state_n != pattern_cell->group_states_used-1)
              {
                append_string(&cell_string, new_string(", "));
              }
            }

            if (pattern_cell->group_states_used > 1)
            {
              append_string(&cell_string, new_string("]"));
            }
          } break;
          case (PatternCellStateType::NOT_STATE):
          {
            CellState state = pattern_cell->states[0];
            String state_name = get_state_name(&rule_config->named_states, state);
            append_string(&cell_string, new_string("!"));
            append_string(&cell_string, state_name);
          } break;
          case (PatternCellStateType::OR_STATE):
          {
            CellState state = pattern_cell->states[0];
            String state_name = get_state_name(&rule_config->named_states, state);
            append_string(&cell_string, new_string("("));
            append_string(&cell_string, state_name);
            append_string(&cell_string, new_string(")"));
          } break;
        }
      }

      u32 buffer_length = cell_string.current_position - cell_string.start;
      assert(buffer_length <= max_state_length);
      fprintf(file_stream, "%-*.*s  ", max_state_length, buffer_length, cell_string.start);
    }

    fprintf(file_stream, "\n");
  }
  fprintf(file_stream, "\n\n");
}


void
serialise_rule_patterns(FILE *file_stream, RuleConfiguration *rule_config)
{
  RulePatterns *rule_patterns = &rule_config->rule_patterns;

  for (u32 rule_pattern_n = 0;
       rule_pattern_n < rule_patterns->n_elements;
       ++rule_pattern_n)
  {
    RulePattern *rule_pattern = rule_patterns->get(rule_pattern_n);
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
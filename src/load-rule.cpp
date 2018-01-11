#include "load-rule.h"

#include "types.h"
#include "text.h"
#include "print.h"
#include "files.h"
#include "parsing.h"
#include "allocate.h"
#include "extendable-array.h"

#include "rule.h"
#include "named-states.h"

/// @file
/// @brief  Loading and reading a .rule file into a Rule struct
///


b32
read_count_matching_value(NamedStates *named_states, String *count_matching_string, RulePattern *rule_pattern_result)
{
  b32 success = true;
  rule_pattern_result->count_matching.enabled = false;

  // Read states group between []
  consume_until_char(count_matching_string, '[');
  String group_string;
  group_string.start = count_matching_string->current_position + 1;
  group_string.current_position = group_string.start;

  // Find end of group
  consume_until_char(count_matching_string, ']');
  group_string.end = count_matching_string->current_position;

  if (group_string.end == count_matching_string->end)
  {
    print("Error: Couldn't find cell state group in count matching in pattern.\n");
    success &= false;
  }
  else
  {
    ExtendableArray<CellState> states;
    states.allocate_array();
    read_named_states_list(named_states, group_string, &states);
    if (states.n_elements > MAX_PATTERN_STATES_GROUP)
    {
      states.un_allocate_array();
      success &= false;
      print("Error: Too many states in count matching group.\n");
    }
    else
    {
      // Copy contents of the temporary Extendable array into count_matching.states
      memcpy(&rule_pattern_result->count_matching.states, states.elements, states.n_elements*states.element_size);
      rule_pattern_result->count_matching.group_states_used = states.n_elements;

      states.un_allocate_array();

      consume_until_char(count_matching_string, ',');

      if (count_matching_string->current_position == count_matching_string->end)
      {
        print("Error in count_matching rule matching state.\n");
        success &= false;
      }
      else
      {
        consume_until(count_matching_string, is_comparison_op);
        if (count_matching_string->current_position[0] == '>')
        {
          rule_pattern_result->count_matching.comparison = ComparisonOp::GREATER_THAN;
        }
        else if (count_matching_string->current_position[0] == '<')
        {
          rule_pattern_result->count_matching.comparison = ComparisonOp::LESS_THAN;
        }
        else if (count_matching_string->current_position[0] == '=')
        {
          rule_pattern_result->count_matching.comparison = ComparisonOp::EQUALS;
        }
        else
        {
          print("Error in count_matching rule count comparison.\n");
          success &= false;
        }

        if (success)
        {
          consume_until(count_matching_string, is_num);
          if (count_matching_string->current_position == count_matching_string->end)
          {
            print("Error in count_matching rule comparison number.\n");
            success &= false;
          }
          else
          {
            rule_pattern_result->count_matching.comparison_n = get_u32(count_matching_string);
            rule_pattern_result->count_matching.enabled = true;
          }
        }
      }
    }
  }

  return success;
}


b32
is_pattern_cell_state_start_character(char character)
{
  b32 result = character == '*' || character == '[' || is_state_character(character);
  return result;
}


b32
read_rule_pattern(NamedStates *named_states, String *file_string, u32 n_inputs, RulePattern *rule_pattern_result)
{
  b32 success = true;

  String line = {};
  String label = {};

  b32 found_pattern = false;
  while (!found_pattern)
  {
    line = get_line(file_string);
    label.start = line.start;

    consume_while(&line, is_label_char);
    label.end = line.current_position;

    if (string_equals(label, "Result:"))
    {
      found_pattern = true;
    }

    if (file_string->current_position == file_string->end)
    {
      break;
    }
    else
    {
      // Move past \n character.
      consume_while(file_string, is_newline);
    }
  }

  file_string->current_position = line.end;

  if (!found_pattern)
  {
    success &= false;
  }
  else
  {
    // Get result value
    consume_until_char(&line, ':');
    if (line.current_position == line.end)
    {
      success &= false;
      return success;
    }
    ++line.current_position;

    String state_name_string = {.start = line.current_position,
                                .current_position = line.current_position,
                                .end = line.end};
    success &= read_state_name(named_states, &state_name_string, &rule_pattern_result->result);
    if (!success)
    {
      print("Error in rule pattern's result value.\n");
      return success;
    }

    // Bounds for the whole of this pattern rule
    String pattern_block = {
      .start = line.end,
      .current_position = line.end,
      .end = file_string->end
    };

    // Find next blank line, i.e: '\n\n', to set the end of the pattern_block to.
    b32 found_block_end = false;
    while (!found_block_end)
    {
      consume_until(&pattern_block, is_newline);

      if (pattern_block.current_position[1] == '\n' || pattern_block.current_position == pattern_block.end)
      {
        pattern_block.end = pattern_block.current_position;
        found_block_end = true;
      }
      else
      {
        ++pattern_block.current_position;
      }
    }

    pattern_block.current_position = pattern_block.start;
    file_string->current_position = pattern_block.end;

    // Get pattern of cells

    for (u32 cell_n = 0;
         cell_n < n_inputs;
         ++cell_n)
    {
      consume_until(&pattern_block, is_pattern_cell_state_start_character);

      if (pattern_block.current_position == pattern_block.end)
      {
        success &= false;
        break;
      }
      else
      {
        PatternCellState *this_cell_state_pattern = rule_pattern_result->cell_states + cell_n;

        if (pattern_block.current_position[0] == '[')
        {
          this_cell_state_pattern->type = PatternCellStateType::STATE;

          String group_string;
          group_string.start = pattern_block.current_position + 1;
          group_string.current_position = group_string.start;

          // Find end of group
          consume_until_char(&pattern_block, ']');
          group_string.end = pattern_block.current_position;

          if (group_string.end == pattern_block.end)
          {
            print("Error: Couldn't find end of cell state group in pattern.\n");
            success &= false;
            break;
          }
          else
          {
            this_cell_state_pattern->group_states_used = 0;
            b32 got_state = true;
            while (got_state && this_cell_state_pattern->group_states_used < MAX_PATTERN_STATES_GROUP)
            {
              CellState *state_slot = this_cell_state_pattern->states + this_cell_state_pattern->group_states_used;
              got_state &= read_state_name(named_states, &group_string, state_slot);

              if (got_state)
              {
                this_cell_state_pattern->group_states_used += 1;
              }
            }
          }
        }
        else if (pattern_block.current_position[0] == '*')
        {
          this_cell_state_pattern->type = PatternCellStateType::WILDCARD;
          ++pattern_block.current_position;
          this_cell_state_pattern->group_states_used = 0;
        }
        else
        {
          this_cell_state_pattern->type = PatternCellStateType::STATE;
          success &= read_state_name(named_states, &pattern_block, &this_cell_state_pattern->states[0]);
          this_cell_state_pattern->group_states_used = 1;

          if (!success)
          {
            print("Error in rule pattern's pattern.\n");
            break;
          }
        }
      }
    }

    // Get any optional labels for this pattern

    String count_matching_string = {};
    b32 count_matching_exists = find_label_value(pattern_block, "count_matching", &count_matching_string);
    if (count_matching_exists)
    {
      success &= read_count_matching_value(named_states, &count_matching_string, rule_pattern_result);
    }
    else
    {
      rule_pattern_result->count_matching.enabled = false;
    }
  }

  return success;
}


b32
read_rule_patterns(NamedStates *named_states, String file_string, u32 n_inputs, RulePatterns *rule_patterns)
{
  b32 success = true;

  RulePattern *rule_pattern = (RulePattern *)allocate_size(rule_patterns->element_size, 1);

  while (file_string.current_position != file_string.end)
  {
    *rule_pattern = {};

    b32 found_pattern = read_rule_pattern(named_states, &file_string, n_inputs, rule_pattern);
    if (found_pattern)
    {
      rule_patterns->add(rule_pattern);
    }
    else
    {
      print("Error in rule pattern.\n");
    }
  }

  un_allocate(rule_pattern);

  return success;
}


b32
read_rule_neighbourhood_region_shape_value(String neighbourhood_region_shape_string, NeighbourhoodRegionShape *result)
{
  b32 success = true;

  if (string_equals(neighbourhood_region_shape_string, "MOORE"))
  {
    *result = NeighbourhoodRegionShape::MOORE;
  }
  else if (string_equals(neighbourhood_region_shape_string, "VON_NEUMANN"))
  {
    *result = NeighbourhoodRegionShape::VON_NEUMANN;
  }
  else if (string_equals(neighbourhood_region_shape_string, "ONE_DIM"))
  {
    *result = NeighbourhoodRegionShape::ONE_DIM;
  }
  else
  {
    success = false;
  }

  return success;
}


b32
load_rule_file(const char *filename, RuleConfiguration *rule_config)
{
  b32 success = true;

  File file;
  success &= open_file(filename, &file);
  if (success)
  {
    String file_string = {
      .start = file.read_ptr,
      .current_position = file.read_ptr,
      .end = file.read_ptr + file.size
    };

    u32 n_states = 0;
    find_label_value_u32(file_string, "n_states", &n_states);
    b32 states_success = n_states > 0;

    if (states_success)
    {
      if (rule_config->named_states.states.elements == 0)
      {
        rule_config->named_states.states.allocate_array();
      }
      rule_config->named_states.states.clear_array();
      rule_config->named_states.next_unused_state = 0;
      states_success &= find_state_names(file_string, &rule_config->named_states, n_states);
    }

    success &= states_success;

    String neighbourhood_region_shape_string = {};
    b32 neighbourhood_region_shape_found = find_label_value(file_string, "neighbourhood_region_shape", &neighbourhood_region_shape_string);
    if (neighbourhood_region_shape_found)
    {
      success &= read_rule_neighbourhood_region_shape_value(neighbourhood_region_shape_string, &rule_config->neighbourhood_region_shape);
    }
    else
    {
      print("No neighbourhood_region_shape supplied.\n");
      success &= false;
    }

    rule_config->neighbourhood_region_size = 0;
    find_label_value_u32(file_string, "neighbourhood_region_size", &rule_config->neighbourhood_region_size);

    String null_states_string = {};
    b32 null_states_found = find_label_value(file_string, "null_states", &null_states_string);
    if (null_states_found && states_success)
    {
      if (rule_config->null_states.elements == 0)
      {
        rule_config->null_states.allocate_array();
      }
      rule_config->null_states.clear_array();

      read_named_states_list(&rule_config->named_states, null_states_string, &rule_config->null_states);
      if (rule_config->null_states.n_elements == 0)
      {
        print("Error in null_states.\n");
        success &= false;
      }
    }
    else
    {
      rule_config->null_states.n_elements = 0;
    }

    success &= rule_config->neighbourhood_region_size > 0;

    if (!success)
    {
      print("Invalid rule file: \"%s\".  Missing or invalid header details.\n", filename);
    }
    else
    {
      debug_print_named_states(&rule_config->named_states);

      print("neighbourhood_region_shape: %s\n", rule_config->neighbourhood_region_shape == NeighbourhoodRegionShape::VON_NEUMANN ? "VON_NEUMANN" : "MOORE");
      print("neighbourhood_region_size: %d\n", rule_config->neighbourhood_region_size);
      print("n_null_states: %d\n", rule_config->null_states.n_elements);
      print("null_states:");
      for (u32 i = 0;
           i < rule_config->null_states.n_elements;
           ++i)
      {
        print(" %d", *rule_config->null_states.get(i));
      }
      print("\n");

      u32 n_inputs = get_neighbourhood_region_n_cells(rule_config->neighbourhood_region_shape, rule_config->neighbourhood_region_size);

      // Need to reallocate, as the size of RulePattern (dependant on n_inputs) might have changed
      rule_config->rule_patterns.un_allocate_array();
      rule_config->rule_patterns.element_size = sizeof(RulePattern) + (sizeof(PatternCellState) * n_inputs);
      rule_config->rule_patterns.allocate_array();

      success &= read_rule_patterns(&rule_config->named_states, file_string, n_inputs, &rule_config->rule_patterns);
      if (!success)
      {
        print("Error whilst reading rule patterns.\n");
      }
      else
      {
        print("n_rule_patterns: %d\n", rule_config->rule_patterns.n_elements);
        for (u32 rule_pattern_n = 0;
             rule_pattern_n < rule_config->rule_patterns.n_elements;
             ++rule_pattern_n)
        {
          RulePattern *rule_pattern = rule_config->rule_patterns.get(rule_pattern_n);
          print("Rule Pattern:\n");
          print("  result: %d\n", rule_pattern->result);
          print("  cells: ");
          for (u32 cell_n = 0;
               cell_n < n_inputs;
               ++cell_n)
          {
            PatternCellState cell = rule_pattern->cell_states[cell_n];
            if (cell.type == PatternCellStateType::WILDCARD)
            {
              print("* ");
            }
            else if (cell.type == PatternCellStateType::STATE)
            {
              if (cell.group_states_used > 1) print("[");
              for (u32 group_state_n = 0;
                   group_state_n < cell.group_states_used;
                   ++group_state_n)
              {
                print("%d ", cell.states[group_state_n]);
              }
              if (cell.group_states_used > 1) print("]");
            }
          }
          print("\n");

          if (rule_pattern->count_matching.enabled)
          {
            char comparison;
            switch (rule_pattern->count_matching.comparison)
            {
              case (ComparisonOp::GREATER_THAN):
              {
                comparison = '>';
              } break;
              case (ComparisonOp::LESS_THAN):
              {
                comparison = '<';
              } break;
              case (ComparisonOp::EQUALS):
              {
                comparison = '=';
              } break;
            }

            print("  count_matching: [");
            for (u32 count_matching_state_n = 0;
                 count_matching_state_n < rule_pattern->count_matching.group_states_used;
                 ++count_matching_state_n)
            {
              print("%d", rule_pattern->count_matching.states[count_matching_state_n]);
            }
            print("], %c %d\n", comparison, rule_pattern->count_matching.comparison_n);
          }
        }
      }
    }

    close_file(&file);
  }

  return success;
}
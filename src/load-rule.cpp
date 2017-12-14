#include "load-rule.h"

#include "types.h"
#include "text.h"
#include "print.h"
#include "files.h"
#include "parsing.h"
#include "allocate.h"
#include "extendable-array.h"

#include "rule.h"


b32
read_count_matching_value(String *count_matching_string, RulePattern *result)
{
  b32 success = true;
  result->count_matching_enabled = false;

  consume_until(count_matching_string, is_num);
  result->count_matching_state = get_u32(count_matching_string);

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
      result->count_matching_comparison = ComparisonOp::GREATER_THAN;
    }
    else if (count_matching_string->current_position[0] == '<')
    {
      result->count_matching_comparison = ComparisonOp::LESS_THAN;
    }
    else if (count_matching_string->current_position[0] == '=')
    {
      result->count_matching_comparison = ComparisonOp::EQUALS;
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
        result->count_matching_n = get_u32(count_matching_string);
        result->count_matching_enabled = true;
      }
    }
  }

  return success;
}


b32
is_cell_state_or_wildcard(char character)
{
  b32 result = character == '*' || is_num(character);
  return result;
}


b32
read_rule_pattern(String *file_string, u32 n_states, u32 n_inputs, RulePattern *result)
{
  b32 success = true;

  String line = {};
  String label = {};

  b32 found_pattern = false;
  while (!found_pattern)
  {
    line = get_line(file_string);

    consume_until(&line, is_letter);
    label.start = line.current_position;

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

    consume_until(&line, is_num);
    if (line.current_position == line.end)
    {
      success &= false;
      return success;
    }

    result->result = get_u32(&line);

    // Bounds for the whole of this pattern rule
    String pattern_block = {
      .start = line.start,
      .current_position = line.end,
      .end = file_string->end
    };

    // Find next blank line, i.e: '\n\n'
    b32 found_block_end = false;
    while (!found_block_end)
    {
      consume_until(&pattern_block, is_newline);

      if (pattern_block.current_position[1] == '\n')
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

    // Get pattern of cells

    for (u32 cell_n = 0;
         cell_n < n_inputs;
         ++cell_n)
    {
      consume_until(file_string, is_cell_state_or_wildcard);

      if (file_string->current_position == file_string->end)
      {
        success &= false;
        break;
      }
      else
      {
        if (file_string->current_position[0] == '*')
        {
          result->cell_states[cell_n].wildcard = true;
          ++file_string->current_position;
        }
        else
        {
          result->cell_states[cell_n].wildcard = false;
          result->cell_states[cell_n].state = get_u32(file_string);
        }
      }
    }

    // Get any optional labels for this pattern

    String count_matching_string = {};
    b32 count_matching_exists = find_label_value(pattern_block, "count_matching", &count_matching_string);
    if (count_matching_exists)
    {
      success &= read_count_matching_value(&count_matching_string, result);
    }
  }

  return success;
}


b32
read_rule_patterns(String *file_string, u32 n_states, u32 n_inputs, ExtendableArray *rule_patterns)
{
  b32 success = true;

  RulePattern *rule_pattern = (RulePattern *)allocate_size(rule_patterns->element_size, 1);

  while (file_string->current_position != file_string->end)
  {

    b32 found_pattern = read_rule_pattern(file_string, n_states, n_inputs, rule_pattern);
    if (found_pattern)
    {
      add_to_extendable_array(rule_patterns, rule_pattern);
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

    rule_config->n_states = 0;
    find_label_value_u32(file_string, "n_states", &rule_config->n_states);

    String neighbourhood_region_shape_string = {};
    b32 neighbourhood_region_shape_found = find_label_value(file_string, "neighbourhood_region_shape", &neighbourhood_region_shape_string);
    if (neighbourhood_region_shape_found)
    {
      success &= read_rule_neighbourhood_region_shape_value(neighbourhood_region_shape_string, &rule_config->neighbourhood_region_shape);
    }
    else
    {
      success &= false;
    }

    rule_config->neighbourhood_region_size = 0;
    find_label_value_u32(file_string, "neighbourhood_region_size", &rule_config->neighbourhood_region_size);

    String null_states_string = {};
    b32 null_states_found = find_label_value(file_string, "null_states", &null_states_string);
    if (null_states_found)
    {
      rule_config->n_null_states = read_u32_list(null_states_string, &rule_config->null_states);
      if (rule_config->n_null_states == 0)
      {
        success &= false;
      }
    }
    else
    {
      rule_config->n_null_states = 0;
    }

    success &= rule_config->neighbourhood_region_size > 0;
    success &= rule_config->n_states > 0;

    if (!success)
    {
      print("Invalid rule file: \"%s\".  Missing or invalid header details.\n", filename);
    }
    else
    {
      print("n_states: %d\n", rule_config->n_states);
      print("neighbourhood_region_shape: %s\n", rule_config->neighbourhood_region_shape == NeighbourhoodRegionShape::VON_NEUMANN ? "VON_NEUMANN" : "MOORE");
      print("neighbourhood_region_size: %d\n", rule_config->neighbourhood_region_size);
      print("n_null_states: %d\n", rule_config->n_null_states);
      print("null_states: ", rule_config->n_null_states);
      for (u32 i = 0;
           i < rule_config->n_null_states;
           ++i)
      {
        print(" %d", rule_config->null_states[i]);
      }
      print("\n");

      u32 n_inputs = get_neighbourhood_region_n_cells(rule_config->neighbourhood_region_shape, rule_config->neighbourhood_region_size) + 1;  // Plus one for centre cell

      new_extendable_array(sizeof(RulePattern) + (sizeof(CellStateWildcard) * n_inputs), &rule_config->rule_patterns);

      success &= read_rule_patterns(&file_string, rule_config->n_states, n_inputs, &rule_config->rule_patterns);
      if (!success)
      {
        print("Error whilst reading rule patterns.\n");
      }
      else
      {
        for (u32 rule_pattern_n = 0;
             rule_pattern_n < rule_config->rule_patterns.next_free_element_position;
             ++rule_pattern_n)
        {
          RulePattern *rule_pattern = (RulePattern *)get_from_extendable_array(&rule_config->rule_patterns, rule_pattern_n);
          print("Rule Pattern:\n");
          print("  result: %d\n", rule_pattern->result);
          print("  cells: ");
          for (u32 cell_n = 0;
               cell_n < n_inputs;
               ++cell_n)
          {
            CellStateWildcard cell = rule_pattern->cell_states[cell_n];
            if (cell.wildcard)
            {
              print("* ");
            }
            else
            {
              print("%d ", cell.state);
            }
          }
          print("\n");

          if (rule_pattern->count_matching_enabled)
          {
            char comparison;
            switch (rule_pattern->count_matching_comparison)
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

            print("  count_matching: %d, %c %d\n", rule_pattern->count_matching_state, comparison, rule_pattern->count_matching_n);
          }
        }
      }
    }

    close_file(&file);
  }

  return success;
}
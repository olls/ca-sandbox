#include "ca-sandbox/load-rule.h"

#include "engine/types.h"
#include "engine/maths.h"
#include "engine/text.h"
#include "engine/print.h"
#include "engine/files.h"
#include "engine/parsing.h"
#include "engine/allocate.h"
#include "engine/my-array.h"

#include "ca-sandbox/rule.h"
#include "ca-sandbox/named-states.h"

/// @file
/// @brief  Loading and reading a .rule file into a Rule struct
///


/// Parses a group of states from the string.  A group contains one or more named state; if it
///   contains one state, the state can be listed naked (i.e: STATE_NAME ).  If the group contains
///   more than one state, the list of states should be enclosed in square brackets, and separated
///   with whitespace (i.e: [STATE_0 STATE_1 STATE_2] ).
///
b32
read_states_group(NamedStates *named_states, String *string, CellStateGroup *group_states_result)
{
  b32 success = true;

  consume_while(string, is_whitespace);

  if (string->current_position[0] != '[')
  {
    // Just reading a single state into states[0]

    success &= read_state_name(named_states, string, &group_states_result->states[0]);
    group_states_result->states_used = 1;
  }
  else
  {
    String group_string = {
      .start = string->current_position + 1,
    };

    // Find end of group
    consume_until_char(string, ']');
    group_string.end = string->current_position;

    group_string.current_position = group_string.start;

    if (group_string.end == string->end)
    {
      print("Error: Couldn't find cell state group end.\n");
      success &= false;
    }
    else
    {
      // Use temporary Array to read in the states
      Array::Array<CellState> states = {};
      read_named_states_list(named_states, group_string, &states);

      if (states.n_elements > MAX_PATTERN_STATES_GROUP)
      {
        success &= false;
        print("Error: Too many states in count matching group.\n");
      }
      else
      {
        memcpy(group_states_result->states, states.elements, states.n_elements * sizeof(CellState));
        group_states_result->states_used = states.n_elements;
      }

      Array::free_array(states);
    }
  }

  return success;
}


b32
read_count_matching_value(NamedStates *named_states, String *count_matching_string, RulePattern *rule_pattern_result)
{
  b32 success = true;
  rule_pattern_result->count_matching.enabled = false;

  b32 states_group_success = read_states_group(named_states, count_matching_string, &rule_pattern_result->count_matching.states_group);

  if (!states_group_success)
  {
    success &= false;
    print("Error: Couldn't read states group in pattern count matching.\n");
  }
  else
  {
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
        if (count_matching_string->current_position[1] == '=')
        {
          rule_pattern_result->count_matching.comparison = ComparisonOp::GREATER_THAN_EQUAL;
        }
        else
        {
          rule_pattern_result->count_matching.comparison = ComparisonOp::GREATER_THAN;
        }
      }
      else if (count_matching_string->current_position[0] == '<')
      {
        if (count_matching_string->current_position[1] == '=')
        {
          rule_pattern_result->count_matching.comparison = ComparisonOp::LESS_THAN_EQUAL;
        }
        else
        {
          rule_pattern_result->count_matching.comparison = ComparisonOp::LESS_THAN;
        }
      }
      else if (count_matching_string->current_position[0] == '=')
      {
        rule_pattern_result->count_matching.comparison = ComparisonOp::EQUAL;
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

  return success;
}


/// Read a single PatternCellState from pattern_block.
///
/// This could be one of:
/// - '*'         Wildcard
/// - '(states)'  OR: Match if one OR pattern cell matches
/// - !states     All except this state
/// - states      Match state
/// Where 'states' is either a single state name, or a group of state names enclosed within '[]'
///
b32
read_rule_pattern_cell(NamedStates *named_states, String *pattern_block, PatternCellState *this_cell_state_pattern)
{
  b32 success = true;

  b32 read_following_states_group = false;
  String states_group_bounds = {
    .start = pattern_block->current_position,
    .end = pattern_block->end
  };

  if (pattern_block->current_position[0] == '*')
  {
    this_cell_state_pattern->type = PatternCellStateType::WILDCARD;
    read_following_states_group = false;
    ++pattern_block->current_position;
    this_cell_state_pattern->states_group.states_used = 0;
  }
  else if (pattern_block->current_position[0] == '(')
  {
    // TODO: Need better name than OR_STATE

    this_cell_state_pattern->type = PatternCellStateType::OR_STATE;
    read_following_states_group = true;

    states_group_bounds.start = pattern_block->current_position + 1;
    states_group_bounds.current_position = states_group_bounds.start;

    // Find end of group
    consume_until_char(pattern_block, ')');
    states_group_bounds.end = pattern_block->current_position;

    if (states_group_bounds.end == pattern_block->end)
    {
      print("Error: Couldn't find end ')' of 'or' cell state in pattern.\n");
      success &= false;
    }
  }
  else if (pattern_block->current_position[0] == '!')
  {
    this_cell_state_pattern->type = PatternCellStateType::NOT_STATE;
    read_following_states_group = true;
    // Cannot constrain states_group_bounds for not pattern
  }
  else
  {
    this_cell_state_pattern->type = PatternCellStateType::STATE;
    read_following_states_group = true;
  }

  if (read_following_states_group)
  {
    states_group_bounds.current_position = states_group_bounds.start;

    b32 states_group_success = read_states_group(named_states, &states_group_bounds, &this_cell_state_pattern->states_group);
    if (!states_group_success)
    {
      print("Error: Couldn't find end of cell state group in pattern.\n");
      success &= false;
    }
    else
    {
      pattern_block->current_position = max(pattern_block->current_position, states_group_bounds.current_position);
    }
  }

  return success;
}


b32
is_pattern_cell_state_start_character(char character)
{
  b32 result = character == '*' || character == '[' || character == '(' || character == '!' || is_state_character(character);
  return result;
}


/// Reads n_inputs number of PatternCellStates sequentially from the pattern_block, storing them in
/// rule_pattern_result.
///
b32
read_rule_pattern_cells(NamedStates *named_states, String pattern_block, u32 n_inputs, RulePattern *rule_pattern_result)
{
  b32 success = true;

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
      success &= read_rule_pattern_cell(named_states, &pattern_block, this_cell_state_pattern);
      if (!success)
      {
        break;
      }
    }
  }

  return success;
}


/// Parses a single RulePattern struct from the current position in the file_string.
///
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

    // Pattern marks the start of a RulePattern definition
    if (string_equals(label, "Pattern:"))
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


  if (!found_pattern)
  {
    file_string->current_position = line.end;
    success &= false;
  }
  else
  {
    consume_until_char(&line, ':');
    // Move past ":"
    ++line.current_position;
    consume_while(&line, is_whitespace);

    // Read comment from after "Pattern: "
    if (line.current_position == line.end)
    {
      rule_pattern_result->comment[0] = '\0';
    }
    else
    {
      u32 comment_length = line.end - line.current_position;
      comment_length = min(comment_length, MAX_COMMENT_LENGTH);
      copy_string(rule_pattern_result->comment, line.current_position, comment_length);
      rule_pattern_result->comment[comment_length] = '\0';
    }

    // Get "Result:" label
    line = get_line(file_string);
    label.start = line.start;

    consume_while(&line, is_label_char);
    label.end = line.current_position;

    file_string->current_position = line.end;

    if (!string_equals(label, "Result:"))
    {
      print("Error: \"Pattern:\" defined without \"Result:\"\n");
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

      // Find bounds for the whole of this RulePattern
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
      success &= read_rule_pattern_cells(named_states, pattern_block, n_inputs, rule_pattern_result);

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
      Array::add(*rule_patterns, rule_pattern);
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


void
debug_print_neighbourhood_region(RuleConfiguration *rule_config)
{
  print("neighbourhood_region_shape: %s\n", rule_config->neighbourhood_region_shape == NeighbourhoodRegionShape::VON_NEUMANN ? "VON_NEUMANN" : "MOORE");
  print("neighbourhood_region_size: %d\n", rule_config->neighbourhood_region_size);
}


void
debug_print_null_states(RuleConfiguration *rule_config)
{
  print("n_null_states: %d\n", rule_config->null_states.n_elements);
  print("null_states:");
  for (u32 i = 0;
       i < rule_config->null_states.n_elements;
       ++i)
  {
    print(" %d", rule_config->null_states[i]);
  }
  print("\n");
}


void
debug_print_rule_patterns(RuleConfiguration *rule_config)
{
  u32 n_inputs = get_neighbourhood_region_n_cells(rule_config->neighbourhood_region_shape, rule_config->neighbourhood_region_size);

  print("n_rule_patterns: %d\n", rule_config->rule_patterns.n_elements);
  for (u32 rule_pattern_n = 0;
       rule_pattern_n < rule_config->rule_patterns.n_elements;
       ++rule_pattern_n)
  {
    RulePattern *rule_pattern = Array::get(rule_config->rule_patterns, rule_pattern_n);
    print("Rule Pattern:\n");
    print("  result: %d\n", rule_pattern->result);
    print("  cells: ");
    for (u32 cell_n = 0;
         cell_n < n_inputs;
         ++cell_n)
    {
      b32 print_group = false;
      const char *prefix = "";
      const char *postfix = "";

      PatternCellState cell = rule_pattern->cell_states[cell_n];
      if (cell.type == PatternCellStateType::WILDCARD)
      {
        print("* ");
      }
      else if (cell.type == PatternCellStateType::STATE)
      {
        print_group = true;
      }
      else if (cell.type == PatternCellStateType::NOT_STATE)
      {
        print_group = true;
        prefix = "!";
        postfix = "";
      }
      else if (cell.type == PatternCellStateType::OR_STATE)
      {
        print_group = true;
        prefix = "(";
        postfix = ")";
      }

      print("%s", prefix);
      if (print_group)
      {
        if (cell.states_group.states_used > 1)
        {
          print("[");
        }
        for (u32 group_state_n = 0;
             group_state_n < cell.states_group.states_used;
             ++group_state_n)
        {
          String state_name = get_state_name(&rule_config->named_states, cell.states_group.states[group_state_n]);
          print("%.*s ", string_length(state_name), state_name.start);
        }
        if (cell.states_group.states_used > 1)
        {
          print("]");
        }
      }
      print("%s ", postfix);
    }
    print("\n");

    if (rule_pattern->count_matching.enabled)
    {
      const char *comparison = COMPARISON_OPERATOR_STRINGS[(u32)rule_pattern->count_matching.comparison];

      print("  count_matching: [");
      for (u32 count_matching_state_n = 0;
           count_matching_state_n < rule_pattern->count_matching.states_group.states_used;
           ++count_matching_state_n)
      {
        print("%d", rule_pattern->count_matching.states_group.states[count_matching_state_n]);
      }
      print("], %s %d\n", comparison, rule_pattern->count_matching.comparison_n);
    }
  }
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
      Array::clear(rule_config->named_states.states);
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
      Array::clear(rule_config->null_states);

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
      debug_print_neighbourhood_region(rule_config);
      debug_print_null_states(rule_config);

      // Parse the RulePatterns
      //

      u32 n_inputs = get_neighbourhood_region_n_cells(rule_config->neighbourhood_region_shape, rule_config->neighbourhood_region_size);

      // Need to reallocate, as the size of RulePattern (dependant on n_inputs) might have changed
      free_array(rule_config->rule_patterns);
      rule_config->rule_patterns.element_size = sizeof(RulePattern) + (sizeof(PatternCellState) * n_inputs);

      success &= read_rule_patterns(&rule_config->named_states, file_string, n_inputs, &rule_config->rule_patterns);
      if (!success)
      {
        print("Error whilst reading rule patterns.\n");
      }
      else
      {
        debug_print_rule_patterns(rule_config);
      }
    }

    close_file(&file);
  }

  return success;
}
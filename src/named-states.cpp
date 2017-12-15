#include "named-states.h"

#include "types.h"
#include "text.h"
#include "print.h"
#include "allocate.h"
#include "parsing.h"

#include "rule.h"
#include "cell.h"


/// Searches the array of state names in the rule configuration for state_name, retuning the
///   corresponding CellState via *resulting_state.  Function returns false, if the name is a
///   defined state name.
///
b32
state_value_from_name(RuleConfiguration *rule_config, String state_name, CellState *resulting_state)
{
  b32 result = false;

  for (u32 test_state_index = 0;
       test_state_index < rule_config->n_states;
       ++test_state_index)
  {
    String *test_state_name = rule_config->state_names + test_state_index;
    if (strings_equal(&state_name, test_state_name))
    {
      *resulting_state = (CellState)test_state_index;
      result = true;
      break;
    }
  }

  return result;
}


b32
read_state_name(RuleConfiguration *rule_config, String *string, CellState *resulting_state_value)
{
  String state_name = {};

  consume_until(string, is_letter);
  state_name.start = string->current_position;

  consume_while(string, is_letter);
  state_name.end = string->current_position;

  b32 result = state_value_from_name(rule_config, state_name, resulting_state_value);
  return result;
}


b32
find_state_names(String file_string, RuleConfiguration *rule_config)
{
  b32 success = true;

  rule_config->state_names = allocate(String, rule_config->n_states);

  u32 n_states_found = 0;
  while (n_states_found < rule_config->n_states)
  {
    String state_name = {};
    b32 found_state_name = find_label_value(file_string, "State", &state_name);

    // Move current position to state end so we don't read this state again
    file_string.current_position = state_name.end;

    if (found_state_name)
    {
      // Check name isn't in use
      b32 name_unique = true;
      for (u32 test_state_index = 0;
           test_state_index < n_states_found;
           ++test_state_index)
      {
        String *test_state_name = rule_config->state_names + test_state_index;
        if (strings_equal(test_state_name, &state_name))
        {
          name_unique = false;
          break;
        }
      }

      if (!name_unique)
      {
        print("Duplicate state names found: \"%.*s\"\n", string_length(state_name), state_name.start);
        success &= false;
        break;
      }
      else
      {
        // Allocate new memory to store the state name in
        char *state_name_text = allocate(char, string_length(state_name));
        copy_string(state_name_text, state_name.start, string_length(state_name));

        rule_config->state_names[n_states_found].start = state_name_text;
        rule_config->state_names[n_states_found].end = state_name_text + string_length(state_name);

        ++n_states_found;
      }
    }
    else
    {
      break;
    }
  }

  if (n_states_found != rule_config->n_states)
  {
    print("Error whilst parsing rule, incorrect number of named rules.\n");
    success &= false;
  }

  return success;
}


u32
read_named_states_list(RuleConfiguration *rule_config, String states_list_string, CellState **resulting_states)
{
  u32 result = 0;

  // Use ExtendibleArray while we build the states list, as we don't know it's length until it is
  //   finished.
  ExtendibleArray states_array = {};
  new_extendible_array(sizeof(CellState), &states_array);

  while (states_list_string.current_position < states_list_string.end)
  {
    String current_state_string = {};

    consume_until(&states_list_string, is_letter);
    current_state_string.start = states_list_string.current_position;

    consume_while(&states_list_string, is_letter);
    current_state_string.end = states_list_string.current_position;

    CellState named_state_value;
    b32 valid_state = state_value_from_name(rule_config, current_state_string, &named_state_value);

    if (valid_state)
    {
      add_to_extendible_array(&states_array, &named_state_value);
      ++result;
    }
  }

  // Because we are using a CellState as the size of the extendible array elements, the
  //   implementation allows us to cast extendible_array.elements to a CellState[] and use it as a
  //   normal array.  This allows us to use the dynamic allocation of the ExtendibleArray whilst
  //   building it, but we can use it as a normal array afterwards.

  *resulting_states = (CellState *)states_array.elements;
  return result;
}
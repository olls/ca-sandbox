#include "named-states.h"

#include "types.h"
#include "text.h"
#include "print.h"
#include "allocate.h"
#include "parsing.h"
#include "extendable-array.h"

#include "cell.h"


/// Searches the array of state names in the rule configuration for state_name, retuning the
///   corresponding CellState via *resulting_state.  Function returns false, if the name is a
///   defined state name.
///
b32
state_value_from_name(NamedStates *named_states, String state_name, CellState *resulting_state)
{
  b32 result = false;

  for (u32 test_state_index = 0;
       test_state_index < named_states->n_states;
       ++test_state_index)
  {
    String *test_state_name = named_states->state_names + test_state_index;
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
is_state_character(char character)
{
  b32 result = is_letter(character) || is_num(character) || character == '_';
  return result;
}


b32
read_state_name(NamedStates *named_states, String *string, CellState *resulting_state_value)
{
  String state_name = {};

  consume_until(string, is_state_character);
  state_name.start = string->current_position;

  consume_while(string, is_state_character);
  state_name.end = string->current_position;

  b32 result = state_value_from_name(named_states, state_name, resulting_state_value);
  return result;
}


b32
find_state_names(String file_string, NamedStates *named_states)
{
  b32 success = true;

  named_states->state_names = allocate(String, named_states->n_states);

  u32 n_states_found = 0;
  while (n_states_found < named_states->n_states)
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
        String *test_state_name = named_states->state_names + test_state_index;
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

        named_states->state_names[n_states_found].start = state_name_text;
        named_states->state_names[n_states_found].end = state_name_text + string_length(state_name);

        ++n_states_found;
      }
    }
    else
    {
      break;
    }
  }

  if (n_states_found != named_states->n_states)
  {
    print("Error whilst parsing rule, incorrect number of named rules.\n");
    success &= false;
  }

  return success;
}


u32
read_named_states_list(NamedStates *named_states, String states_list_string, CellState **resulting_states)
{
  u32 result = 0;

  // Use ExtendableArray while we build the states list, as we don't know it's length until it is
  //   finished.
  ExtendableArray states_array = {};
  new_extendable_array(sizeof(CellState), &states_array);

  while (states_list_string.current_position < states_list_string.end)
  {
    CellState named_state_value;
    b32 valid_state = read_state_name(named_states, &states_list_string, &named_state_value);

    if (valid_state)
    {
      add_to_extendable_array(&states_array, &named_state_value);
      ++result;
    }
  }

  // Because we are using a CellState as the size of the extendable array elements, the
  //   implementation allows us to cast extendable_array.elements to a CellState[] and use it as a
  //   normal array.  This allows us to use the dynamic allocation of the ExtendableArray whilst
  //   building it, but we can use it as a normal array afterwards.

  *resulting_states = (CellState *)states_array.elements;
  return result;
}


void
debug_print_named_states(NamedStates *named_states)
{
  print("n_states: %d\n", named_states->n_states);
  print("states:");
  for (u32 i = 0;
       i < named_states->n_states;
       ++i)
  {
    String state_name = named_states->state_names[i];
    print(" %.*s", string_length(state_name), state_name.start);
  }
  print("\n");
}
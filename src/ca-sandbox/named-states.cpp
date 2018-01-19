#include "named-states.h"

#include "util.h"
#include "types.h"
#include "text.h"
#include "print.h"
#include "allocate.h"
#include "maths.h"
#include "parsing.h"
#include "extendable-array.h"

#include "cell.h"

/// @file
/// @brief  Functions for parsing and identifying state names
///


CellState
get_next_unused_state_value(NamedStates *named_states)
{
  CellState result;
  result = named_states->next_unused_state++;
  return result;
}


/// Searches the array of state names in the rule configuration for state_name, retuning the
///   corresponding CellState via *resulting_state.  Function returns false, if the name is a
///   defined state name.
///
b32
state_value_from_name(NamedStates *named_states, String state_name, CellState *resulting_state)
{
  b32 result = false;

  for (u32 test_state_index = 0;
       test_state_index < named_states->states.n_elements;
       ++test_state_index)
  {
    NamedState *test_state_name = named_states->states.get(test_state_index);
    if (strings_equal(&state_name, &test_state_name->name))
    {
      *resulting_state = test_state_name->value;
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
find_state_names(String file_string, NamedStates *named_states, u32 n_states)
{
  b32 success = true;

  while (named_states->states.n_elements < n_states)
  {
    String label_value = {};
    b32 found_state_name = find_label_value(file_string, "State", &label_value);

    String state_name = {};
    consume_until(&label_value, is_state_character);
    state_name.start = label_value.current_position;

    consume_while(&label_value, is_state_character);
    state_name.end = label_value.current_position;

    // Move current position to state end so we don't read this state again
    file_string.current_position = state_name.end;

    if (found_state_name)
    {
      // Check name isn't in use
      b32 name_unique = true;
      for (u32 test_state_index = 0;
           test_state_index < named_states->states.n_elements;
           ++test_state_index)
      {
        NamedState *test_state_name = named_states->states.get(test_state_index);
        if (strings_equal(&test_state_name->name, &state_name))
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

        NamedState new_named_state = {
          .name = {
            .start = state_name_text,
            .end = state_name_text + string_length(state_name)
          },
          .value = get_next_unused_state_value(named_states)
        };

        named_states->states.add(new_named_state);
      }
    }
    else
    {
      break;
    }
  }

  if (named_states->states.n_elements != n_states)
  {
    print("Error whilst parsing rule, incorrect number of named rules.\n");
    success &= false;
  }

  return success;
}


void
read_named_states_list(NamedStates *named_states, String states_list_string, ExtendableArray<CellState> *resulting_states)
{
  while (states_list_string.current_position < states_list_string.end)
  {
    CellState named_state_value;
    b32 valid_state = read_state_name(named_states, &states_list_string, &named_state_value);

    if (valid_state)
    {
      resulting_states->add(named_state_value);
    }
  }
}


void
debug_print_named_states(NamedStates *named_states)
{
  print("n_states: %d\n", named_states->states.n_elements);
  print("states:");
  for (u32 i = 0;
       i < named_states->states.n_elements;
       ++i)
  {
    NamedState *state_name = named_states->states.get(i);
    print(" %.*s", string_length(state_name->name), state_name->name.start);
  }
  print("\n");
}


String
get_state_name(NamedStates *named_states, CellState state)
{
  String result = {};

  // The state values do not correspond to the states positions in named_states

  for (u32 state_index = 0;
       state_index < named_states->states.n_elements;
       ++state_index)
  {
    NamedState *named_state = named_states->states.get(state_index);

    if (named_state->value == state)
    {
      result = named_state->name;
      break;
    }
  }

  return result;
}


CellState
advance_state(NamedStates *named_states, CellState previous)
{
  CellState result;

  if (previous == named_states->states.n_elements - 1)
  {
    result = 0;
  }
  else
  {
    result = previous + 1;
  }

  return result;
}


vec4
get_state_colour(CellState state)
{
  vec4 colours[] = {(vec4){0x60/255.0, 0x60/255.0, 0x60/255.0, 1},
                    (vec4){0xff/255.0, 0xA0/255.0, 0xA0/255.0, 1},
                    (vec4){0xff/255.0, 0x7d/255.0, 0x00/255.0, 1},
                    (vec4){0xff/255.0, 0x96/255.0, 0x19/255.0, 1},
                    (vec4){0xff/255.0, 0xaf/255.0, 0x32/255.0, 1},
                    (vec4){0xff/255.0, 0xc8/255.0, 0x4b/255.0, 1},
                    (vec4){0xff/255.0, 0xe1/255.0, 0x64/255.0, 1},
                    (vec4){0xff/255.0, 0xfa/255.0, 0x7d/255.0, 1},
                    (vec4){0xfb/255.0, 0xff/255.0, 0x00/255.0, 1},
                    (vec4){0x59/255.0, 0x59/255.0, 0xff/255.0, 1},
                    (vec4){0x6a/255.0, 0x6a/255.0, 0xff/255.0, 1},
                    (vec4){0x7a/255.0, 0x7a/255.0, 0xff/255.0, 1},
                    (vec4){0x8b/255.0, 0x8b/255.0, 0xff/255.0, 1},
                    (vec4){0x1b/255.0, 0xb0/255.0, 0x1b/255.0, 1},
                    (vec4){0x24/255.0, 0xc8/255.0, 0x24/255.0, 1},
                    (vec4){0x49/255.0, 0xff/255.0, 0x49/255.0, 1},
                    (vec4){0x6a/255.0, 0xff/255.0, 0x6a/255.0, 1},
                    (vec4){0xeb/255.0, 0x24/255.0, 0x24/255.0, 1},
                    (vec4){0xff/255.0, 0x38/255.0, 0x38/255.0, 1},
                    (vec4){0xff/255.0, 0x49/255.0, 0x49/255.0, 1},
                    (vec4){0xff/255.0, 0x59/255.0, 0x59/255.0, 1},
                    (vec4){0xb9/255.0, 0x38/255.0, 0xff/255.0, 1},
                    (vec4){0xbf/255.0, 0x49/255.0, 0xff/255.0, 1},
                    (vec4){0xc5/255.0, 0x59/255.0, 0xff/255.0, 1},
                    (vec4){0xcb/255.0, 0x6a/255.0, 0xff/255.0, 1},
                    (vec4){0x00/255.0, 0xff/255.0, 0x80/255.0, 1},
                    (vec4){0xff/255.0, 0x80/255.0, 0x40/255.0, 1},
                    (vec4){0xff/255.0, 0xff/255.0, 0x80/255.0, 1},
                    (vec4){0x21/255.0, 0xd7/255.0, 0xd7/255.0, 1},
                    (vec4){0x1b/255.0, 0xb0/255.0, 0xb0/255.0, 1},
                    (vec4){0x18/255.0, 0x9c/255.0, 0x9c/255.0, 1},
                    (vec4){0x15/255.0, 0x89/255.0, 0x89/255.0, 1}};

  vec4 colour = colours[state % array_count(colours)];
  return colour;
}


u32
get_longest_state_name_length(NamedStates *named_states)
{
  u32 result = 0;

  for (u32 state_n = 0;
       state_n < named_states->states.n_elements;
       ++state_n)
  {
    NamedState *named_state = named_states->states.get(state_n);
    result = max(result, string_length(named_state->name));
  }

  return result;
}

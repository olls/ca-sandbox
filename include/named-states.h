#ifndef NAMED_STATES_H_DEF
#define NAMED_STATES_H_DEF

#include "types.h"
#include "text.h"
#include "my-array.h"

#include "cell.h"

/// @file
/// @brief  Managing named states
///


struct NamedState
{
  String name;
  CellState value;
};


struct NamedStates
{
  Array::Array<NamedState> states;
  CellState next_unused_state;
};


CellState
get_next_unused_state_value(NamedStates *named_states);


b32
is_state_character(char character);


b32
read_state_name(NamedStates *named_states, String *string, CellState *resulting_state_value);


b32
find_state_names(String file_string, NamedStates *named_states, u32 n_states);


void
read_named_states_list(NamedStates *named_states, String states_list_string, Array::Array<CellState> *resulting_states);


void
debug_print_named_states(NamedStates *named_states);


String
get_state_name(NamedStates *named_states, CellState state);


CellState
advance_state(NamedStates *named_states, CellState previous);


vec4
get_state_colour(CellState state);


u32
get_longest_state_name_length(NamedStates *named_states);


#endif
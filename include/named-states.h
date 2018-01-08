#ifndef NAMED_STATES_H_DEF
#define NAMED_STATES_H_DEF

#include "types.h"
#include "text.h"

#include "cell.h"

/// @file
/// @brief  Managing named states
///


struct NamedStates
{
  /// Number of cell states the rule uses
  u32 n_states;

  /// Array of state names, indexed by state value
  String *state_names;
};


b32
is_state_character(char character);


b32
read_state_name(NamedStates *named_states, String *string, CellState *resulting_state_value);


b32
find_state_names(String file_string, NamedStates *named_states);


u32
read_named_states_list(NamedStates *named_states, String null_states_string, CellState **resulting_states);


void
debug_print_named_states(NamedStates *named_states);


CellState
advance_state(NamedStates *named_states, CellState previous);


#endif
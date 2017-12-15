#ifndef NAMED_STATES_H_DEF
#define NAMED_STATES_H_DEF

#include "types.h"
#include "text.h"

#include "rule.h"


b32
is_state_character(char character);


b32
read_state_name(RuleConfiguration *rule_config, String *string, CellState *resulting_state_value);


b32
find_state_names(String file_string, RuleConfiguration *rule_config);


u32
read_named_states_list(RuleConfiguration *rule_config, String null_states_string, CellState **resulting_states);


#endif
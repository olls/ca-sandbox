#ifndef LOAD_RULE_H_DEF
#define LOAD_RULE_H_DEF

#include "types.h"

#include "rule.h"
#include "cell.h"


struct CellStateWildcard
{
  b32 wildcard;
  CellState state;
};


enum struct ComparisonOp
{
  GREATER_THAN,
  LESS_THAN,
  EQUALS
};


struct RulePattern
{
  CellState result;

  b32 count_matching_enabled;
  CellState count_matching_state;
  ComparisonOp count_matching_comparison;
  u32 count_matching_n;

  CellStateWildcard cell_states[];
};


b32
load_rule_file(const char *filename, RuleConfiguration *rule_configuration);


#endif
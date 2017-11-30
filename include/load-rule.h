#ifndef LOAD_RULE_H_DEF
#define LOAD_RULE_H_DEF

#include "types.h"

#include "rule.h"
#include "cell.h"


// Rule pattern matching:
// - Rules are defined case by case, with pattern matching to simplify and reduce the number of
//     cases needed.
// - A rule pattern is defined with:
//       Result: s
//     Where s is the resulting state, if this pattern matches.
// - Following this is the pattern, written out as each cell-state in the neighbourhood region
//     (including the center), left-to-right, top-to-bottom, separated by whitespace.  Cells can be
//     represented by a wildcard: '*' if the state value does not need to be matched for this
//     pattern.
// - Optional conditions can be added after the pattern:
//   - count_matching: [n], [cond]
//     The pattern only matches if the count of wildcard states equalling n compared with cond is
//       true.  (Useful for Game Of Life like CAs.)
//     The condition is represented by one of >, <, =, followed by the number to compare against.


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
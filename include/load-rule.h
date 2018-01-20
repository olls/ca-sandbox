#ifndef LOAD_RULE_H_DEF
#define LOAD_RULE_H_DEF

#include "types.h"
#include "comparison-operator.h"

#include "cell.h"
#include "neighbourhood-region.h"
#include "named-states.h"

/// @file
/// @brief  structs for loading a .rule file into a Rule struct
///
/// Rule pattern matching:
/// - Rules are defined case by case, with pattern matching to simplify and reduce the number of
///     cases needed.
/// - A rule pattern is defined with:
///       Result: s
///     Where s is the resulting state, if this pattern matches.
/// - Following this is the pattern, written out as each cell-state in the neighbourhood region
///     (including the centre), left-to-right, top-to-bottom, separated by whitespace.  Cells can be
///     represented by a wildcard: '*' if the state value does not need to be matched for this
///     pattern.
/// - Optional conditions can be added after the pattern:
///   - count_matching: [n], [cond]
///     The pattern only matches if the count of wildcard states equalling n compared with cond is
///       true.  (Useful for Game Of Life like CAs.)
///     The condition is represented by one of >, <, =, followed by the number to compare against.


enum struct PatternCellStateType : s32
{
  WILDCARD,
  STATE,
  NOT_STATE,
  OR_STATE // TODO: Better name?
};


static const char *PATTERN_CELL_STATE_TYPE_NAMES[] = {
  "Wildcard state",
  "Match one of n states",
  "Match all state except",
  "\"OR\" State"
};


/// Maximum number of state values which can be grouped together in a CellStateGroup
const u32 MAX_PATTERN_STATES_GROUP = 16;


/// Fixed size group of CellStates, used for RulePattern.count_matching.states_group and
///   PatternCellState.states_group
///
struct CellStateGroup
{
  CellState states[MAX_PATTERN_STATES_GROUP];
  u32 states_used;
};


struct PatternCellState
{
  PatternCellStateType type;
  CellStateGroup states_group;
};


struct CountMatching
{
  b32 enabled;
  CellStateGroup states_group;
  ComparisonOp comparison;
  u32 comparison_n;
};


const u32 MAX_COMMENT_LENGTH = 512;

struct RulePattern
{
  char comment[MAX_COMMENT_LENGTH];
  CellState result;

  CountMatching count_matching;

  /// Left-to-right, top-to-bottom list of cell states in pattern
  PatternCellState cell_states[];
};


typedef ExtendableArray<RulePattern> RulePatterns;


/// Holds the configuration of a rule, i.e: the parsed data from the rule file.
///
/// - NULL states indicate states which do not need simulating, these are used so an INFINITE border
///     simulation can still run in finite time.
///   - CellBlocks must be initialised with NULL states.
///   - NULL state rules must be stable (i.e. No change in state) when inputs in neighbourhood
///       region are also NULL states.
///   - This allows the simulator to avoid simulating/creating a CellBlock if:
///     - Its initial state is all NULL states
///     - It is still in its initial state
///     - There are no non-NULL state Cells within the rule neighbourhood of the CellBlock's border.
///
struct RuleConfiguration
{
  NeighbourhoodRegionShape neighbourhood_region_shape;

  /// The distance the neighbourhood region extends from the centre cell, see
  ///   get_neighbourhood_region_n_cells() for definitions
  u32 neighbourhood_region_size;

  NamedStates named_states;

  /// Array of state values which are NULL states
  ExtendableArray<CellState> null_states;

  RulePatterns rule_patterns;
};


b32
load_rule_file(const char *filename, RuleConfiguration *rule_config);


#endif
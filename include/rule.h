#ifndef RULE_H_DEF
#define RULE_H_DEF

#include "extendable-array.h"
#include "text.h"

#include "cell.h"
#include "border.h"
#include "neighbourhood-region.h"
#include "named-states.h"


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
  CellState *null_states;
  /// Length of null_states
  u32 n_null_states;

  ExtendableArray rule_patterns;
};


struct RuleNode
{
  /// If the node is a leaf, then the traversal stops here, and the output value is stored in the
  ///   leaf_output_value member.
  b32 is_leaf;
  CellState leaf_value;

  /// This array should have a size of Rule.n_states
  ///
  /// Each entries position in the array represents a possible state of the current neighbour points
  ///   to the next node if the position matches the current state of
  ///   this neighbour
  ///
  u32 children[];
};


/// - A Rule is represented by a tree data structure, where each node represents the state of a
///     neighbour, and the child to look at for the next neighbour.  The tree has a (max) depth of
///     the number of inputs to the transition function, i.e: the number of neighbour nodes +1 for
///     the central node.  (Some branches of the tree may stop at a lower depth, as the rest of the
///     neighbours may not needed)
///
/// - To use the rule you traverse down the tree.  At each node, you select the next child node
///     based on the current neighbour's state, then advance to the next neighbour as you traverse
///     to the next tree node.  To start the traversal you start at the root node, and the central
///     cell.
///
/// - The use of a tree allows the direct representation of all the nodes, whilst keeping the memory
///     usage viable as identical sub-trees are represented by the same memory.
///
struct Rule
{
  RuleConfiguration config;

  /// Number of inputs to the transition function, i.e: number of neighbours + centre cell
  u32 n_inputs;

  /// Array of all RuleNodes making up this rule.
  ExtendableArray rule_nodes_table;

  /// Position of the rule tree's root node within the rule_nodes_table.
  u32 root_node;
};


b32
is_null_state(RuleConfiguration *rule_configuration, CellState state);


void
build_rule_tree(Rule *result);


void
destroy_rule_tree(Rule *rule);


void
print_rule_tree(Rule *rule_tree);


CellState
execute_transition_function(Border *border, Universe *universe, Rule *rule, s32vec2 cell_block_position, s32vec2 cell_position);


#endif
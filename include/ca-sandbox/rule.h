#ifndef RULE_H_DEF
#define RULE_H_DEF

#include "engine/my-array.h"
#include "engine/text.h"

#include "ca-sandbox/cell.h"
#include "ca-sandbox/border.h"
#include "ca-sandbox/load-rule.h"

#include <pthread.h>

/// @file
/// @brief  structs for storing a rule
///


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

  /// Indicates whether the rule tree has been built
  b32 rule_tree_built;

  /// Number of inputs to the transition function, i.e: number of neighbours + centre cell
  u32 n_inputs;

  /// Array of all RuleNodes making up this rule.
  Array::Array<RuleNode, true> rule_nodes_table;

  /// Position of the rule tree's root node within the rule_nodes_table.
  u32 root_node;
};


struct Progress
{
  u64 total;
  u64 done;
};


struct RuleCreationThread
{
  Rule *rule;

  b32 currently_running;
  pthread_t thread;

  u32 last_build_total_time;

  Progress progress;
};


b32
is_null_state(RuleConfiguration *rule_configuration, CellState state);


b32
start_build_rule_tree_thread(RuleCreationThread *rule_creation_thread, Rule *result);


void
destroy_rule_tree(Rule *rule);


void
print_rule_tree(Rule *rule_tree);


CellState
execute_transition_function(Border *border, Universe *universe, Rule *rule, s32vec2 cell_block_position, s32vec2 cell_position);


#endif
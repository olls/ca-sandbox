#ifndef RULE_H_DEF
#define RULE_H_DEF

#include "extendable-array.h"

#include "cell.h"


enum struct NeighbourhoodRegionShape
{
  VON_NEUMANN,
  MOORE,
  ONE_DIM
};


struct RuleConfiguration
{
  NeighbourhoodRegionShape neighbourhood_region_shape;
  u32 neighbourhood_region_size;
  u32 n_states;

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


/// A Rule is represented by a tree data structure, where each node represents the state of a
///   neighbour, and the child to look at for the next neighbour.  The tree has a (max) depth of the
///   number of inputs to the transition function, i.e: the number of neighbour nodes +1 for the
///   central node.  (Some branches of the tree may stop at a lower depth, as the rest of the
///   neighbours may not needed)
///
/// To use the rule you traverse down the tree.  At each node, you select the next child node based
///   on the current neighbour's state, then advance to the next neighbour as you traverse to the
///   next tree node.  To start the traversal you start at the root node, and the central cell.
///
/// The use of a tree allows the direct representation of all the nodes, whilst keeping the memory
///   usage viable as identical sub-trees are represented by the same memory.
///
struct Rule
{
  RuleConfiguration config;

  /// Number of inputs to the transition function, i.e: number of neighbours + centre cell
  u32 n_inputs;

  /// Size in bytes of one RuleNode for this n_states value
  u32 rule_node_size;

  u32 rule_nodes_table_size;
  u32 next_free_rule_node_position;
  RuleNode *rule_nodes_table;

  u32 root_node;
};


u32
get_neighbourhood_region_n_cells(NeighbourhoodRegionShape shape, u32 size);


void
build_rule_tree(Rule *result);


void
print_rule_tree(Rule *rule_tree);


CellState
execute_transition_function(Rule *rule, CellState *cell_states);


#endif
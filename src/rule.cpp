#include <string.h>

#include "rule.h"

#include "allocate.h"
#include "assert.h"
#include "print.h"
#include "maths.h"


u32
get_neighbourhood_region_n_cells(NeighbourhoodRegionShape shape, u32 size)
{
  u32 result = 0;

  switch (shape)
  {
    case (NeighbourhoodRegionShape::VON_NEUMANN):
    {
      // 1 -> 4
      //   #
      // #   #
      //   #
      //
      // 2 -> 8
      //     #
      //     #
      // # #   # #
      //     #
      //     #
      result = size * 4;
    } break;

    case (NeighbourhoodRegionShape::MOORE):
    {
      // 1 -> 8
      // # # #
      // #   #
      // # # #
      //
      // 2 -> 24
      // # # # # #
      // # # # # #
      // # #   # #
      // # # # # #
      // # # # # #
      result = pow(((size*2) + 1), 2) - 1;
    } break;

    case (NeighbourhoodRegionShape::ONE_DIM):
    {
      // 1 -> 2
      // #   #
      //
      // 2 -> 4
      // # #   # #
      result = size * 2;
    } break;
  }

  return result;
}


u32
new_node(Rule *rule)
{
  if (rule->next_free_rule_node_position == rule->rule_nodes_table_size)
  {
    rule->rule_nodes_table_size *= 2;
    rule->rule_nodes_table = (RuleNode *)re_allocate(rule->rule_nodes_table, rule->rule_node_size * rule->rule_nodes_table_size);
  }

  assert(rule->rule_nodes_table != 0);

  u32 position = rule->next_free_rule_node_position;

  ++rule->next_free_rule_node_position;

  return position;
}


RuleNode *
get_rule_node(Rule *rule, u32 index)
{
  // Have to do this indexing in bytes, because the RuleNodes are a variable sized struct.
  return (RuleNode *)((char *)(rule->rule_nodes_table) + (index * rule->rule_node_size));
}


s32
find_node(Rule *rule, RuleNode *node)
{
  s32 result = -1;

  for (u32 node_n = 0;
       node_n < rule->next_free_rule_node_position;
       ++node_n)
  {
    RuleNode *test_node = get_rule_node(rule, node_n);

    if (node->is_leaf && test_node->is_leaf &&
        node->leaf_value == test_node->leaf_value)
    {
      result = node_n;
      break;
    }
    else if (!node->is_leaf && !test_node->is_leaf)
    {
      b32 all_children_match = true;
      for (u32 child_n = 0;
           child_n < rule->config.n_states;
           ++child_n)
      {
        if (node->children[child_n] != test_node->children[child_n])
        {
          all_children_match = false;
          break;
        }
      }

      if (all_children_match)
      {
        result = node_n;
        break;
      }
    }
  }

  return result;
}


u32
add_node_to_rule_tree(Rule *rule, u32 depth, CellState tree_path[])
{
  print("\n%*s", 2*depth, "");

  u32 node_position;

  // Temporary storage for the node
  RuleNode *node = (RuleNode *)allocate_size(rule->rule_node_size, 1);

  if (depth == rule->n_inputs)
  {
    node->is_leaf = true;
    node->leaf_value = rule->config.transition_function(rule->n_inputs, tree_path);

    print("Leaf(%d)", node->leaf_value);
  }
  else
  {
    print("Node");

    node->is_leaf = false;

    // Generate all children by iterating over all states for this level/neighbour
    for (CellState child_n = 0;
         child_n < rule->config.n_states;
         ++child_n)
    {
      tree_path[depth] = child_n;

      u32 child_position = add_node_to_rule_tree(rule, depth + 1, tree_path);
      node->children[child_n] = child_position;
    }
  }

  // Optimise node

  s32 existing_node_position = find_node(rule, node);
  if (existing_node_position >= 0)
  {
    node_position = existing_node_position;
  }
  else
  {
    node_position = new_node(rule);
    RuleNode *new_node = get_rule_node(rule, node_position);

    memcpy(new_node, node, rule->rule_node_size);

    u32 a = 0;
  }

  print("\n%*sEnding node %d", 2*depth, "", node_position);

  un_allocate(node);

  return node_position;
}


void
build_rule_tree(RuleConfiguration rule_configuration, Rule *result)
{
  print("\nConstructing Rule Tree\n");

  result->config = rule_configuration;

  result->rule_node_size = sizeof(RuleNode) + (result->config.n_states * sizeof(u32));

  result->rule_nodes_table_size = 16;
  result->rule_nodes_table = (RuleNode *)allocate_size(result->rule_node_size, result->rule_nodes_table_size);
  result->next_free_rule_node_position = 0;

  result->n_inputs = get_neighbourhood_region_n_cells(result->config.neighbourhood_region_shape, result->config.neighbourhood_region_size) + 1;

  CellState *tree_path = allocate(CellState, result->n_inputs);
  result->root_node = add_node_to_rule_tree(result, 0, tree_path);

  print("\n");
}


void
print_node(Rule *rule, u32 node_position, u32 depth)
{
  print("\n%*sposition(%d): ", 2*depth, "", node_position);

  RuleNode *rule_node = get_rule_node(rule, node_position);

  if (rule_node->is_leaf)
  {
    print("Leaf(%d)", rule_node->leaf_value);
  }
  else
  {
    print("Node");
    for (CellState child_n = 0;
         child_n < rule->config.n_states;
         ++child_n)
    {
      print_node(rule, rule_node->children[child_n], depth+1);
    }
  }
}


void
print_rule_tree(Rule *rule_tree)
{
  print("\nPrinting Rule Tree\n");

  print_node(rule_tree, rule_tree->root_node, 0);
}


CellState
rule30_transition_function(u32 n_inputs, CellState inputs[])
{
  CellState result = inputs[1];

  assert(n_inputs == 3);

  // 111 110 101 100 011 010 001 000
  // 0   0   0   1   1   1   1   0

  if (inputs[0] == 1 &&
      inputs[1] == 1 &&
      inputs[2] == 1)
  {
    result = 0;
  }
  else if (inputs[0] == 1 &&
           inputs[1] == 1 &&
           inputs[2] == 0)
  {
    result = 0;
  }
  else if (inputs[0] == 1 &&
           inputs[1] == 0 &&
           inputs[2] == 1)
  {
    result = 0;
  }
  else if (inputs[0] == 1 &&
           inputs[1] == 0 &&
           inputs[2] == 0)
  {
    result = 1;
  }
  else if (inputs[0] == 0 &&
           inputs[1] == 1 &&
           inputs[2] == 1)
  {
    result = 1;
  }
  else if (inputs[0] == 0 &&
           inputs[1] == 1 &&
           inputs[2] == 0)
  {
    result = 1;
  }
  else if (inputs[0] == 0 &&
           inputs[1] == 0 &&
           inputs[2] == 1)
  {
    result = 1;
  }
  else if (inputs[0] == 0 &&
           inputs[1] == 0 &&
           inputs[2] == 0)
  {
    result = 0;
  }

  return result;
}


void
dummy_make_rule30_rule_tree(Rule *result)
{
  RuleConfiguration rule_configuration;
  rule_configuration.n_states = 2;
  rule_configuration.neighbourhood_region_shape = NeighbourhoodRegionShape::ONE_DIM;
  rule_configuration.neighbourhood_region_size = 1;

  rule_configuration.transition_function = rule30_transition_function;

  build_rule_tree(rule_configuration, result);

  print_rule_tree(result);
}


void
build_rule_tree()
{

}


void
execute_transition_function(Rule *rule, CellState *cell_states)
{
  // cell_states is an array of all the neighbours top-to-bottom left-to-right, with the central
  //   cell moved to the first position.  (Length = NEIGHBOURHOOD_REGION_SIZES[rule.neighbourhood_region_shape])
  //
  // TODO: Might want to move the getting of the cell-states into here, so that we only get them
  //       when they are needed.





}
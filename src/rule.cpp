#include <string.h>

#include "rule.h"

#include "allocate.h"
#include "assert.h"
#include "print.h"


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


s32
find_node(Rule *rule, RuleNode *node)
{
  s32 result = -1;

  for (u32 node_n = 0;
       node_n < rule->next_free_rule_node_position;
       ++node_n)
  {
    RuleNode *test_node = rule->rule_nodes_table + node_n;

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
           child_n < rule->n_states;
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
    node->leaf_value = rule->transition_function(rule->n_inputs, tree_path);

    print("Leaf(%d)", node->leaf_value);
  }
  else
  {
    print("Node");

    node->is_leaf = false;

    // Generate all children by iterating over all states for this level/neighbour
    for (CellState child_n = 0;
         child_n < rule->n_states;
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
    RuleNode *new_node = rule->rule_nodes_table + node_position;

    if (node->is_leaf)
    {
      new_node->is_leaf = node->is_leaf;
      new_node->leaf_value = node->leaf_value;
    }
    else
    {
      new_node->is_leaf = false;
      memcpy(new_node->children, node->children, rule->n_states*sizeof(u32));
    }
  }

  print("\n%*sEnding node %d", 2*depth, "", node_position);

  un_allocate(node);

  return node_position;
}


void
make_rule_tree(Rule *rule_tree)
{
  print("\nConstructing Rule Tree\n");

  rule_tree->rule_node_size = sizeof(RuleNode) + rule_tree->n_states;

  rule_tree->rule_nodes_table_size = 16;
  rule_tree->rule_nodes_table = (RuleNode *)allocate_size(rule_tree->rule_node_size, rule_tree->rule_nodes_table_size);
  rule_tree->next_free_rule_node_position = 0;

  rule_tree->n_inputs = NEIGHBOURHOOD_REGION_SIZES[(u32)rule_tree->neighbourhood_region_shape] + 1;

  CellState *tree_path = allocate(CellState, rule_tree->n_inputs);
  rule_tree->root_node = add_node_to_rule_tree(rule_tree, 0, tree_path);

  print("\n");
}


void
print_node(Rule *rule, u32 node_position, u32 depth)
{
  print("\n%*sposition(%d): ", 2*depth, "", node_position);

  RuleNode *rule_node = rule->rule_nodes_table + node_position;

  if (rule_node->is_leaf)
  {
    print("Leaf(%d)", rule_node->leaf_value);
  }
  else
  {
    print("Node");
    for (CellState child_n = 0;
         child_n < rule->n_states;
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
dummy_make_hardcoded_rule_tree(Rule *result)
{
  result->n_states = 2;
  result->neighbourhood_region_shape = NeighbourhoodRegionShape::ONE_DIM;

  result->transition_function = rule30_transition_function;

  make_rule_tree(result);
  print_rule_tree(result);
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
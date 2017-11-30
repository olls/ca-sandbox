#include <string.h>

#include "rule.h"

#include "allocate.h"
#include "assert.h"
#include "print.h"
#include "maths.h"
#include "extendable-array.h"

#include "load-rule.h"


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


/// Matches the input against the RulePatterns, finds the first matching rule pattern and uses that
///   output.
CellState
use_rule_patterns_to_get_result(RuleConfiguration *config, u32 n_inputs, CellState inputs[])
{
  CellState result;
  b32 found_match = false;

  for (u32 pattern_n = 0;
       pattern_n < config->rule_patterns.next_free_element_position;
       ++pattern_n)
  {
    RulePattern *rule_pattern = (RulePattern *)get_from_extendable_array(&config->rule_patterns, pattern_n);

    b32 matches = true;
    u32 count_matching_state_n = 0;

    for (u32 input_n = 0;
         input_n < n_inputs;
         ++input_n)
    {
      CellState in = inputs[input_n];
      CellStateWildcard pattern_input = rule_pattern->cell_states[input_n];

      if (!pattern_input.wildcard)
      {
        if (pattern_input.state != in)
        {
          matches = false;
          break;
        }
      }
      else if (rule_pattern->count_matching_enabled)
      {
        if (in == rule_pattern->count_matching_state)
        {
          ++count_matching_state_n;
        }
      }
    }

    // Now test the wildcard constraints
    if (matches && rule_pattern->count_matching_enabled)
    {
      if (rule_pattern->count_matching_comparison == ComparisonOp::GREATER_THAN)
      {
        matches = count_matching_state_n > rule_pattern->count_matching_n;
      }
      else if (rule_pattern->count_matching_comparison == ComparisonOp::LESS_THAN)
      {
        matches = count_matching_state_n < rule_pattern->count_matching_n;
      }
      else if (rule_pattern->count_matching_comparison == ComparisonOp::EQUALS)
      {
        matches = count_matching_state_n == rule_pattern->count_matching_n;
      }
    }

    if (matches)
    {
      found_match = true;
      result = rule_pattern->result;
      break;
    }
  }

  if (!found_match)
  {
    // Default rule, keep previous state

    // TODO: Is this always correct?
    u32 center_position = n_inputs / 2;

    result = inputs[center_position];
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
  u32 node_position;

  // Temporary storage for the node
  RuleNode *node = (RuleNode *)allocate_size(rule->rule_node_size, 1);

  if (depth == rule->n_inputs)
  {
    node->is_leaf = true;
    node->leaf_value = use_rule_patterns_to_get_result(&rule->config, rule->n_inputs, tree_path);
  }
  else
  {
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

  un_allocate(node);

  return node_position;
}


/// Set Rule.config values before calling!
void
build_rule_tree(Rule *result)
{
  result->rule_node_size = sizeof(RuleNode) + (result->config.n_states * sizeof(u32));

  result->rule_nodes_table_size = 16;
  result->rule_nodes_table = (RuleNode *)allocate_size(result->rule_node_size, result->rule_nodes_table_size);
  result->next_free_rule_node_position = 0;

  result->n_inputs = get_neighbourhood_region_n_cells(result->config.neighbourhood_region_shape, result->config.neighbourhood_region_size) + 1;

  CellState *tree_path = allocate(CellState, result->n_inputs);
  result->root_node = add_node_to_rule_tree(result, 0, tree_path);
}


void
print_node(Rule *rule, u32 node_position, u32 depth, u32 n_inputs, CellState inputs[])
{
  print("\n%*sposition(%d): ", 2*depth, "", node_position);

  RuleNode *rule_node = get_rule_node(rule, node_position);

  if (rule_node->is_leaf)
  {
    print("Leaf(%d) <- ", rule_node->leaf_value);

    // Print inputs (path to node)
    for (u32 input_n = 0;
         input_n < n_inputs;
         ++input_n)
    {
      print("%d ", inputs[input_n]);
    }
  }
  else
  {
    print("Node");
    for (CellState child_n = 0;
         child_n < rule->config.n_states;
         ++child_n)
    {
      inputs[depth] = child_n;
      print_node(rule, rule_node->children[child_n], depth+1, n_inputs, inputs);
    }
  }
}


void
print_rule_tree(Rule *rule_tree)
{
  print("\nPrinting Rule Tree\n");

  CellState inputs[rule_tree->n_inputs];
  print_node(rule_tree, rule_tree->root_node, 0, rule_tree->n_inputs, inputs);

  print("\n");
}


CellState
execute_transition_function(Rule *rule, CellState *cell_states)
{
  // cell_states is an array of all the neighbours top-to-bottom left-to-right, including the
  //   central cell.

  // TODO: Might want to move the getting of the cell-states into here, so that we only get them
  //       when they are needed.

  CellState result;

  RuleNode *node = get_rule_node(rule, rule->root_node);

  u32 input_n = 0;
  b32 reached_result = false;
  while (!reached_result)
  {
    if (node->is_leaf)
    {
      reached_result = true;
    }
    else
    {
      CellState current_state = cell_states[input_n];
      u32 next_node_position = node->children[current_state];
      node = get_rule_node(rule, next_node_position);
      ++input_n;
    }
  }

  assert(input_n == rule->n_inputs);

  result = node->leaf_value;
  return result;
}
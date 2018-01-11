#include "rule.h"

#include "allocate.h"
#include "assert.h"
#include "print.h"
#include "maths.h"
#include "extendable-array.h"

#include "load-rule.h"
#include "simulate.h"
#include "neighbourhood-region.h"

#include <string.h>
#include "imgui.h"

/// @file
/// @brief  Functions for building a rule tree, and accessing it.
///


b32
is_null_state(RuleConfiguration *rule_configuration, CellState state)
{
  b32 result = false;

  for (u32 null_state_index = 0;
       null_state_index < rule_configuration->null_states.n_elements;
       ++null_state_index)
  {
    CellState null_state = *rule_configuration->null_states.get(null_state_index);
    result |= state == null_state;
  }

  return result;
}


/// Matches the input against the RulePatterns, finds the first matching rule pattern and uses that
///   output.
///
CellState
use_rule_patterns_to_get_result(RuleConfiguration *config, u32 n_inputs, CellState inputs[])
{
  CellState result;
  b32 found_match = false;

  for (u32 pattern_n = 0;
       pattern_n < config->rule_patterns.n_elements;
       ++pattern_n)
  {
    RulePattern *rule_pattern = config->rule_patterns.get(pattern_n);

    b32 matches = true;
    u32 count_matching_state_n = 0;

    for (u32 input_n = 0;
         input_n < n_inputs;
         ++input_n)
    {
      CellState in = inputs[input_n];
      PatternCellState pattern_input = rule_pattern->cell_states[input_n];

      switch (pattern_input.type)
      {
        case (PatternCellStateType::STATE):
        {
          // Look for a matching state in the PatternCellState group
          b32 found_match_in_group = false;

          for (u32 group_state_n = 0;
               group_state_n < pattern_input.group_states_used;
               ++group_state_n)
          {
            if (pattern_input.states[group_state_n] == in)
            {
              found_match_in_group = true;
              break;
            }
          }

          if (!found_match_in_group)
          {
            matches = false;
          }
        } break;

        case (PatternCellStateType::WILDCARD):
        {
          if (rule_pattern->count_matching_enabled &&
              in == rule_pattern->count_matching_state)
          {
            ++count_matching_state_n;
          }
        } break;
      }

      if (!matches)
      {
        break;
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

    u32 center_position = get_neighbourhood_region_centre_index(config->neighbourhood_region_shape, config->neighbourhood_region_size);

    result = inputs[center_position];
  }

  return result;
}


/// Finds and returns the position of an existing RuleNode within the rule storage which matches the
///   RuleNode passed in.
///
s32
find_node(Rule *rule, RuleNode *node)
{
  s32 result = -1;

  for (u32 node_n = 0;
       node_n < rule->rule_nodes_table.n_elements;
       ++node_n)
  {
    RuleNode *test_node = rule->rule_nodes_table.get(node_n);

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
           child_n < rule->config.named_states.states.n_elements;
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
add_node_to_rule_tree(Rule *rule, u32 depth, CellState tree_path[], u64 n_nodes_estimate)
{
  static u64 n_nodes_traced = 0;

  u32 node_position;

  // Temporary storage for the node
  RuleNode *node = (RuleNode *)allocate_size(rule->rule_nodes_table.element_size, 1);

  if (depth == rule->n_inputs)
  {
    node->is_leaf = true;
    node->leaf_value = use_rule_patterns_to_get_result(&rule->config, rule->n_inputs, tree_path);

    u64 intervals = 20;
    if (n_nodes_traced % (n_nodes_estimate / intervals) == 0)
    {
      r64 percent_done = 100 * (r64)n_nodes_traced / (r64)n_nodes_estimate;
      print("Generating rule tree: %.1lf\%, %lu/%lu\n", percent_done, n_nodes_traced, n_nodes_estimate);
    }
    ++n_nodes_traced;
  }
  else
  {
    node->is_leaf = false;

    // Generate all children by iterating over all states for this level/neighbour

    for (CellState child_n = 0;
         child_n < rule->config.named_states.states.n_elements;
         ++child_n)
    {
      // This is the list of inputs for the current child
      tree_path[depth] = child_n;

      u32 child_position = add_node_to_rule_tree(rule, depth + 1, tree_path, n_nodes_estimate);
      node->children[child_n] = child_position;
    }
  }

  // Optimise finished node

  // If it matches any existing nodes, use their index instead of creating a new node.
  s32 existing_node_position = find_node(rule, node);
  if (existing_node_position >= 0)
  {
    node_position = existing_node_position;
  }
  else
  {
    // Create new node
    node_position = rule->rule_nodes_table.add(node);
  }

  un_allocate(node);

  return node_position;
}


/// Set Rule.config values before calling!
void
build_rule_tree(Rule *result)
{
  // We can continue to use the existing table if the element_size (i.e: the n_states) has not
  //   changed, as the tree will be rebuilt out of the existing nodes.  We currently destroy the
  //   rule table before calling build_rule_tree if the file is reloaded to create a new table.
  //   If the tree is altered via the GUI there is no way to change the n_states (currently), so we
  //   don't need a new table.

  if (result->rule_nodes_table.elements == 0)
  {
    result->rule_nodes_table.un_allocate_array();
    u32 n_states = result->config.named_states.states.n_elements;
    result->rule_nodes_table.element_size = sizeof(RuleNode) + (n_states * sizeof(u32));
    result->rule_nodes_table.allocate_array();
  }

  result->n_inputs = get_neighbourhood_region_n_cells(result->config.neighbourhood_region_shape, result->config.neighbourhood_region_size);

  u64 n_nodes_estimate = ipow((u64)result->config.named_states.states.n_elements, (u64)result->n_inputs);
  print("Building rule tree: %lu\n", n_nodes_estimate);

  // The tree_path is used to store the route taken through the tree to reach a leaf node.
  CellState *tree_path = allocate(CellState, result->n_inputs);
  result->root_node = add_node_to_rule_tree(result, 0, tree_path, n_nodes_estimate);

  result->rule_tree_built = true;

  un_allocate(tree_path);
}


void
destroy_rule_tree(Rule *rule)
{
  rule->rule_tree_built = false;
  rule->rule_nodes_table.un_allocate_array();
}


void
print_node(Rule *rule, u32 node_position, u32 depth, CellState inputs[])
{
  print("\n%*sposition(%d): ", 2*depth, "", node_position);

  RuleNode *rule_node = rule->rule_nodes_table.get(node_position);

  if (rule_node->is_leaf)
  {
    print("Leaf(%d) <- ", rule_node->leaf_value);

    // Print inputs (path to node)
    for (u32 input_n = 0;
         input_n < depth;
         ++input_n)
    {
      print("%d ", inputs[input_n]);
    }
  }
  else
  {
    print("Node");
    for (CellState child_n = 0;
         child_n < rule->config.named_states.states.n_elements;
         ++child_n)
    {
      inputs[depth] = child_n;
      print_node(rule, rule_node->children[child_n], depth+1, inputs);
    }
  }
}


void
print_rule_tree(Rule *rule_tree)
{
  print("\nPrinting Rule Tree\n");

  CellState inputs[rule_tree->n_inputs];
  print_node(rule_tree, rule_tree->root_node, 0, inputs);

  print("\n");
}


/// Executes the transition function by traversing the rule tree taking inputs from the universe as
///   needed
///
/// @param[in] border  Universe border config
/// @param[in] universe  The universe
/// @param[in] rule  The rule tree to use
/// @param[in] cell_block_position  The position of the block containing the cell to transition
/// @param[in] cell_position  The position of the cell to transition within the given cell block
/// @param[in] null_state_0  The null state to use with this rule for cells which are not loaded
CellState
execute_transition_function(Border *border, Universe *universe, Rule *rule, s32vec2 cell_block_position, s32vec2 cell_position)
{
  // cell_states is an array of all the neighbours top-to-bottom left-to-right, including the
  //   central cell.

  CellState result;

  RuleNode *node = rule->rule_nodes_table.get(rule->root_node);

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
      s32vec2 current_input_delta = get_neighbourhood_region_cell_delta(rule->config.neighbourhood_region_shape, rule->config.neighbourhood_region_size, input_n);

      CellState current_state;

      // If get_neighbouring_cell_state doesn't set current_state, the cell is not currently loaded
      //   because it is a null state.  Therefore, use the first null state as the current state
      //   instead.  If there are no null states, the cell should always exist as
      //   create_any_new_cell_blocks_needed will create all the cell blocks.
      if (rule->config.null_states.n_elements > 0)
      {
        current_state = *rule->config.null_states.get(0);
      }

      b32 simulate_cell = get_neighbouring_cell_state(border, universe, current_input_delta, cell_block_position, cell_position, &current_state);

      if (!simulate_cell)
      {
        // Neighbour is outside the simulation region border, therefore do not simulate the cell.

        break;
      }

      // Select the next node based on this input's state
      u32 next_node_position = node->children[current_state];
      node = rule->rule_nodes_table.get(next_node_position);
      ++input_n;
    }
  }

  if (reached_result)
  {
    result = node->leaf_value;
  }
  else
  {
    result = DEBUG_STATE;
  }

  return result;
}
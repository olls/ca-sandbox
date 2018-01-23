#include "ca-sandbox/rule.h"

#include "engine/allocate.h"
#include "engine/assert.h"
#include "engine/print.h"
#include "engine/maths.h"
#include "engine/my-array.h"
#include "engine/timing.h"

#include "ca-sandbox/load-rule.h"
#include "ca-sandbox/simulate.h"
#include "ca-sandbox/neighbourhood-region.h"

#include "imgui/imgui.h"

#include <string.h>
#include <pthread.h>

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
    CellState null_state = rule_configuration->null_states[null_state_index];
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
    RulePattern& rule_pattern = config->rule_patterns[pattern_n];

    b32 matches = true;
    u32 number_of_neighbours_matching_count_matching_states = 0;

    b32 or_matching_enabled_on_this_pattern = false;
    u32 number_of_neighbours_matching_or_states = 0;

    for (u32 input_n = 0;
         input_n < n_inputs;
         ++input_n)
    {
      CellState in = inputs[input_n];
      PatternCellState pattern_input = rule_pattern.cell_states[input_n];

      switch (pattern_input.type)
      {
        case (PatternCellStateType::STATE):
        {
          // Match if any states in the group match 'in'
          b32 found_match_in_group = false;

          for (u32 group_state_n = 0;
               group_state_n < pattern_input.states_group.states_used;
               ++group_state_n)
          {
            if (pattern_input.states_group.states[group_state_n] == in)
            {
              found_match_in_group = true;
              break;
            }
          }
          if (found_match_in_group)
          {
            matches = true;
          }
          else
          {
            matches = false;
          }
        } break;

        case (PatternCellStateType::NOT_STATE):
        {
          // Ensure 'in' is not equal to any of the group states
          b32 found_match_in_group = false;

          for (u32 group_state_n = 0;
               group_state_n < pattern_input.states_group.states_used;
               ++group_state_n)
          {
            if (pattern_input.states_group.states[group_state_n] == in)
            {
              found_match_in_group = true;
              break;
            }
          }
          if (found_match_in_group)
          {
            matches = false;
          }
        } break;

        case (PatternCellStateType::OR_STATE):
        {
          // If 'in' matches any of the states in the group for this OR, increment count.
          or_matching_enabled_on_this_pattern = true;

          for (u32 group_state_n = 0;
               group_state_n < pattern_input.states_group.states_used;
               ++group_state_n)
          {
            if (pattern_input.states_group.states[group_state_n] == in)
            {
              number_of_neighbours_matching_or_states += 1;
              // Only need to find one matching state per OR cell state.
              break;
            }
          }
        } break;

        case (PatternCellStateType::WILDCARD):
        {
          if (rule_pattern.count_matching.enabled)
          {
            b32 this_state_in_count_matching_group = false;

            for (u32 count_matching_state_n = 0;
                 count_matching_state_n < rule_pattern.count_matching.states_group.states_used;
                 ++count_matching_state_n)
            {
              if (in == rule_pattern.count_matching.states_group.states[count_matching_state_n])
              {
                this_state_in_count_matching_group = true;
                break;
              }
            }

            if (this_state_in_count_matching_group)
            {
              number_of_neighbours_matching_count_matching_states += 1;
            }
          }
        } break;
      }

      // This input does not match -> early out
      if (!matches)
      {
        break;
      }
    }

    if (matches)
    {
      // Now test the wildcard constraints
      if (rule_pattern.count_matching.enabled)
      {
        if (rule_pattern.count_matching.comparison == ComparisonOp::GREATER_THAN)
        {
          matches = number_of_neighbours_matching_count_matching_states > rule_pattern.count_matching.comparison_n;
        }
        else
        if (rule_pattern.count_matching.comparison == ComparisonOp::GREATER_THAN_EQUAL)
        {
          matches = number_of_neighbours_matching_count_matching_states >= rule_pattern.count_matching.comparison_n;
        }
        else if (rule_pattern.count_matching.comparison == ComparisonOp::EQUAL)
        {
          matches = number_of_neighbours_matching_count_matching_states == rule_pattern.count_matching.comparison_n;
        }
        else if (rule_pattern.count_matching.comparison == ComparisonOp::LESS_THAN_EQUAL)
        {
          matches = number_of_neighbours_matching_count_matching_states <= rule_pattern.count_matching.comparison_n;
        }
        else if (rule_pattern.count_matching.comparison == ComparisonOp::LESS_THAN)
        {
          matches = number_of_neighbours_matching_count_matching_states < rule_pattern.count_matching.comparison_n;
        }
      }

      // If any OR_STATE is used in the pattern, we must have matched against one or more of them
      if (or_matching_enabled_on_this_pattern)
      {
        if (number_of_neighbours_matching_or_states == 0)
        {
          matches = false;
        }
      }

      if (matches)
      {
        // We match this pattern

        found_match = true;
        result = rule_pattern.result;
        break;
      }
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
    RuleNode *test_node = Array::get(rule->rule_nodes_table, node_n);

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
add_node_to_rule_tree(Rule *rule, u32 depth, CellState tree_path[], Array::Array<RuleNode, true>& current_node_path, Progress *progress)
{
  static u64 n_nodes_traced = 0;

  u32 node_position;

  // Temporary storage for the node
  RuleNode *node = Array::get(current_node_path, depth);

  if (depth == rule->n_inputs)
  {
    node->is_leaf = true;
    node->leaf_value = use_rule_patterns_to_get_result(&rule->config, rule->n_inputs, tree_path);

    progress->done += 1;

#if 0
    u64 intervals = 20;
    if (progress->done % (progress->total / intervals) == 0)
    {
      r64 percent_done = 100 * (r64)progress->done / (r64)progress->total;
      print("Generating rule tree: %.1lf\%, %lu/%lu\n", percent_done, progress->done, progress->total);
    }
#endif
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

      u32 child_position = add_node_to_rule_tree(rule, depth + 1, tree_path, current_node_path, progress);
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
    node_position = Array::new_position(rule->rule_nodes_table);
    Array::set(rule->rule_nodes_table, node_position, node);
  }

  return node_position;
}


/// Set Rule.config values before calling!
void
build_rule_tree(RuleCreationThread *rule_creation_thread)
{
  Rule *rule = rule_creation_thread->rule;

  rule->rule_tree_built = false;

  u32 n_states = rule->config.named_states.states.n_elements;
  rule->rule_nodes_table.element_size = sizeof(RuleNode) + (n_states * sizeof(u32));
  Array::free_array(rule->rule_nodes_table);

  rule->n_inputs = get_neighbourhood_region_n_cells(rule->config.neighbourhood_region_shape, rule->config.neighbourhood_region_size);

  rule_creation_thread->progress.total = ipow((u64)rule->config.named_states.states.n_elements, (u64)rule->n_inputs);
  rule_creation_thread->progress.done = 0;
  print("Building rule tree: %lu\n", rule_creation_thread->progress.total);

  u64 build_start_time = get_us();

  // The tree_path is used to store the route taken through the tree to reach a leaf node.
  CellState *tree_path = allocate(CellState, rule->n_inputs);

  // One node per depth of the tree used to store the currently being built nodes to get to the
  //   current position in the tree.  One a node is finished, it is added to the
  //   rule->rule_nodes_table, and the next sibling in the tree is able to use the current_node_path
  //   node it was occupying.  Need n_inputs+1 for leaf "result node".
  //
  // Using an Array to make dealing with variable sized structs easy, need to pre-allocate all the
  //   elements in the array
  Array::Array<RuleNode, true> current_node_path = {};
  current_node_path.element_size = rule->rule_nodes_table.element_size;
  Array::new_position_for_n(current_node_path, rule->n_inputs + 1);

  rule->root_node = add_node_to_rule_tree(rule, 0, tree_path, current_node_path, &rule_creation_thread->progress);

  Array::free_array(current_node_path);
  un_allocate(tree_path);

  rule_creation_thread->last_build_total_time = get_us() - build_start_time;

  rule->rule_tree_built = true;
  rule_creation_thread->currently_running = false;
}


void *
build_rule_tree_thread(void *rule_creation_thread)
{
  build_rule_tree((RuleCreationThread *)rule_creation_thread);

  return NULL;
}


b32
start_build_rule_tree_thread(RuleCreationThread *rule_creation_thread, Rule *result)
{
  b32 success = true;

  if (!rule_creation_thread->currently_running)
  {
    rule_creation_thread->rule = result;

#define RULE_BUILDER_THREADING
#ifdef RULE_BUILDER_THREADING
    s32 error = pthread_create(&rule_creation_thread->thread, NULL, build_rule_tree_thread, (void *)rule_creation_thread);
    if (error)
    {
      success &= false;
    }
    else
    {
      rule_creation_thread->currently_running = true;
    }
#else
    build_rule_tree(rule_creation_thread);
#endif
  }

  return success;
}


void
destroy_rule_tree(Rule *rule)
{
  rule->rule_tree_built = false;
  Array::free_array(rule->rule_nodes_table);
}


void
print_node(Rule *rule, u32 node_position, u32 depth, CellState inputs[])
{
  print("\n%*sposition(%d): ", 2*depth, "", node_position);

  RuleNode& rule_node = rule->rule_nodes_table[node_position];

  if (rule_node.is_leaf)
  {
    print("Leaf(%d) <- ", rule_node.leaf_value);

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
      print_node(rule, rule_node.children[child_n], depth+1, inputs);
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

  RuleNode *node = Array::get(rule->rule_nodes_table, rule->root_node);

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
        current_state = rule->config.null_states[0];
      }

      b32 simulate_cell = get_neighbouring_cell_state(border, universe, current_input_delta, cell_block_position, cell_position, &current_state);

      if (!simulate_cell)
      {
        // Neighbour is outside the simulation region border, therefore do not simulate the cell.

        break;
      }

      // Select the next node based on this input's state
      u32 next_node_position = node->children[current_state];
      node = Array::get(rule->rule_nodes_table, next_node_position);
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
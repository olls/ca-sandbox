#include <string.h>

#include "rule.h"

#include "allocate.h"
#include "assert.h"
#include "print.h"
#include "maths.h"
#include "extendible-array.h"

#include "load-rule.h"
#include "simulate.h"


u32
get_neighbourhood_region_n_cells(NeighbourhoodRegionShape shape, u32 size)
{
  u32 result = 0;

  switch (shape)
  {
    case (NeighbourhoodRegionShape::VON_NEUMANN):
    {
      // 1 -> 5
      //   #
      // # # #
      //   #
      //
      // 2 -> 9
      //     #
      //     #
      // # # # # #
      //     #
      //     #
      result = (size * 4) + 1;
    } break;

    case (NeighbourhoodRegionShape::MOORE):
    {
      // 1 -> 9
      // # # #
      // # # #
      // # # #
      //
      // 2 -> 25
      // # # # # #
      // # # # # #
      // # # # # #
      // # # # # #
      // # # # # #
      result = pow(((size*2) + 1), 2);
    } break;

    case (NeighbourhoodRegionShape::ONE_DIM):
    {
      // 1 -> 3
      // # # #
      //
      // 2 -> 5
      // # # # # #
      result = (size * 2) + 1;
    } break;
  }

  return result;
}



/// Convert neighbour index into direction, from left-to-right, top-to-bottom.  Used to loop over
///   all neighbours in execute_transision_function().
///
s32vec2
get_neighbourhood_region_cell_delta(NeighbourhoodRegionShape shape, u32 size, u32 index)
{
  assert(index < get_neighbourhood_region_n_cells(shape, size));

  s32vec2 result;
  switch (shape)
  {
    case (NeighbourhoodRegionShape::MOORE):
    {
      u32 neighbourhood_region_side_length = ((size * 2) + 1);

      result = (s32vec2){(s32)(index % neighbourhood_region_side_length),
                         (s32)(index / neighbourhood_region_side_length)};

      result = vec2_subtract(result, size);

    } break;
    case (NeighbourhoodRegionShape::VON_NEUMANN):
    {
      u32 distance_from_centre;

      if (index < size)
      {
        // Up spoke
        result = (s32vec2){ 0, -1};
        distance_from_centre = size - index;
      }
      else if (index < 2 * size)
      {
        // Left spoke
        result = (s32vec2){-1,  0};
        distance_from_centre = 2 * size - index;
      }
      else if (index == size * 2)
      {
        // Centre
        result = (s32vec2){0, 0};
        distance_from_centre = 0;
      }
      else if (index < (size * 3) + 1)
      {
        // Right spoke
        result = (s32vec2){ 1,  0};
        distance_from_centre = index - (size * 2);
      }
      else if (index < (size * 4) + 1)
      {
        // Bottom spoke
        result = (s32vec2){ 0,  1};
        distance_from_centre = index - (size * 3);
      }
      else
      {
        print("Error: index passed into get_von_neumann_neighbour_delta is too large for size.\n");
        assert(0);
      }

      result = vec2_multiply(result, distance_from_centre);

    } break;

    case (NeighbourhoodRegionShape::ONE_DIM):
    {
      result = (s32vec2){(s32)(index - size), 0};

    } break;
  }

  return result;
}


b32
is_null_state(RuleConfiguration *rule_configuration, CellState state)
{
  b32 result = false;

  for (u32 null_state_index = 0;
       null_state_index < rule_configuration->n_null_states;
       ++null_state_index)
  {
    CellState null_state = rule_configuration->null_states[null_state_index];
    result |= state == null_state;
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
    RulePattern *rule_pattern = (RulePattern *)get_from_extendible_array(&config->rule_patterns, pattern_n);

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

  result->n_inputs = get_neighbourhood_region_n_cells(result->config.neighbourhood_region_shape, result->config.neighbourhood_region_size);

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
      s32vec2 current_input_delta = get_neighbourhood_region_cell_delta(rule->config.neighbourhood_region_shape, rule->config.neighbourhood_region_size, input_n);

      CellState current_state;

      // If get_neighbouring_cell_state doesn't set current_state, the cell is not currently loaded
      //   because it is a null state.  Therefore, use the first null state as the current state
      //   instead.  If there are no null states, the cell should always exist as
      //   create_any_new_cell_blocks_needed will create all the cell blocks.
      if (rule->config.n_null_states > 0)
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
      node = get_rule_node(rule, next_node_position);
      ++input_n;
    }
  }

  if (reached_result)
  {
    assert(input_n == rule->n_inputs);

    result = node->leaf_value;
  }
  else
  {
    result = DEBUG_STATE;
  }

  return result;
}
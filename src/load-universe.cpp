#include "load-universe.h"

#include "util.h"
#include "print.h"
#include "text.h"
#include "parsing.h"
#include "files.h"
#include "allocate.h"

#include "universe.h"
#include "simulate.h"
#include "named-states.h"


void
read_cell_block(String *file_string, Universe *universe, NamedStates *named_states)
{
  String line;
  String label;

  b32 found_cell_block = false;
  while (!found_cell_block)
  {
    line = get_line(file_string);

    consume_until(&line, is_letter);
    label.start = line.current_position;

    consume_while(&line, is_label_char);
    label.end = line.current_position;

    if (string_equals(label, "CellBlock"))
    {
      found_cell_block = true;
    }

    if (file_string->current_position == file_string->end)
    {
      break;
    }
    else
    {
      // Move past \n character.
      consume_while(file_string, is_newline);
    }
  }

  file_string->current_position = line.end;

  if (found_cell_block)
  {
    consume_until_char(&line, ':');
    if (line.current_position == line.end)
    {
      return;
    }
    ++line.current_position;

    consume_until(&line, is_num_or_sign);
    if (line.current_position == line.end)
    {
      return;
    }

    s32 x = get_s32(&line);

    consume_until(&line, is_num_or_sign);
    if (line.current_position == line.end)
    {
      return;
    }

    s32 y = get_s32(&line);

    print("CellBlock: %d, %d\n", x, y);

    // TODO: Use a block string to contain separate cell blocks, as is done for the patterns.

    CellBlock *cell_block = create_uninitialised_cell_block(universe, (s32vec2){x, y});

    for (u32 cell_index = 0;
         cell_index < universe->cell_block_dim * universe->cell_block_dim;
         ++cell_index)
    {
      Cell *cell = cell_block->cells + cell_index;

      b32 state_read = read_state_name(named_states, file_string, &cell->state);
      if (!state_read)
      {
        print("Invalid state name in cell block.\n");
      }
    }
  }
}


/// Loads a Universe object from a .cell file.

/// @param[in] file_string  String containing the contents of the .cell file
/// @param[out] universe  Universe to fill in
b32
load_universe_from_file(String file_string, Universe *universe, NamedStates *named_states)
{
  b32 success = true;

  String cell_block_dim_string;
  String n_cell_blocks_string;

  success &= find_label_value(file_string, "cell_block_dim", &cell_block_dim_string);
  success &= find_label_value(file_string, "n_cell_blocks", &n_cell_blocks_string);

  s32 cell_block_dim = get_s32(&cell_block_dim_string);
  s32 n_cell_blocks = get_s32(&n_cell_blocks_string);

  if (!success)
  {
    print("Missing/erroneous values in file\n");
  }
  else
  {
    print("cell_block_dim: %d\n", cell_block_dim);
    print("n_cell_blocks: %d\n", n_cell_blocks);

    universe->cell_block_dim = cell_block_dim;

    for (u32 cell_block_index = 0;
         cell_block_index < n_cell_blocks;
         ++cell_block_index)
    {
      read_cell_block(&file_string, universe, named_states);
    }
  }

  return success;
}


b32
read_border_type_value(String border_type_string, BorderType *result)
{
  b32 success = true;

  if (string_equals(border_type_string, "FIXED"))
  {
    *result = BorderType::FIXED;
  }
  else if (string_equals(border_type_string, "TORUS"))
  {
    *result = BorderType::TORUS;
  }
  else if (string_equals(border_type_string, "INFINITE"))
  {
    *result = BorderType::INFINITE;
  }
  else
  {
    success = false;
  }

  return success;
}


void
debug_print_simulate_options(SimulateOptions *simulate_options)
{
  const char *border_types[] = {"FIXED", "INFINITE", "TORUS"};
  print("border.type: %s\n", border_types[(u32)simulate_options->border.type]);

  print("border.min_corner_block: (%d, %d)\n", simulate_options->border.min_corner_block.x, simulate_options->border.min_corner_block.y);
  print("border.min_corner_cell:  (%d, %d)\n", simulate_options->border.min_corner_cell.x, simulate_options->border.min_corner_cell.y);
  print("border.max_corner_block: (%d, %d)\n", simulate_options->border.max_corner_block.x, simulate_options->border.max_corner_block.y);
  print("border.max_corner_cell:  (%d, %d)\n", simulate_options->border.max_corner_cell.x, simulate_options->border.max_corner_cell.y);
}


/// Loads a SimulateOptions object from a .cell file.

/// @param[in] file_string  String containing the contents of the .cell file
/// @param[out] simulate_options  SimulateOptions to fill in (Unspecified values are left alone, so
///                                 fill in with defaults before calling)
b32
load_simulate_options(String file_string, SimulateOptions *simulate_options)
{
  b32 success = true;

  String border_type_string = {};
  String border_min_block_string = {};
  String border_min_cell_string = {};
  String border_max_block_string = {};
  String border_max_cell_string = {};

  b32 border_type_defined = find_label_value(file_string, "border_type", &border_type_string);
  if (border_type_defined)
  {
    success &= read_border_type_value(border_type_string, &simulate_options->border.type);
  }

  b32 border_limits_defined = true;
  border_limits_defined &= find_label_value(file_string, "border_min_block", &border_min_block_string);
  border_limits_defined &= find_label_value(file_string, "border_min_cell", &border_min_cell_string);
  border_limits_defined &= find_label_value(file_string, "border_max_block", &border_max_block_string);
  border_limits_defined &= find_label_value(file_string, "border_max_cell", &border_max_cell_string);

  if (border_limits_defined)
  {
    b32 error_in_border_limits = false;
    error_in_border_limits &= get_vector(border_min_block_string, &simulate_options->border.min_corner_block);
    error_in_border_limits &= get_vector(border_min_cell_string, &simulate_options->border.min_corner_cell);
    error_in_border_limits &= get_vector(border_max_block_string, &simulate_options->border.max_corner_block);
    error_in_border_limits &= get_vector(border_max_cell_string, &simulate_options->border.max_corner_cell);

    if (error_in_border_limits)
    {
      success &= false;
      print("Error in border definitions\n");
    }
  }

  if (success)
  {
      debug_print_simulate_options(simulate_options);
  }
  else
  {
    print("Error whilst parsing simulate options\n");
  }

  return success;
}


b32
read_cell_initialisation_type_value(String intialisation_type_string, CellInitialisationType *result)
{
  b32 success = true;

  if (string_equals(intialisation_type_string, "RANDOM"))
  {
    *result = CellInitialisationType::RANDOM;
  }
  else
  {
    success = false;
  }

  return success;
}


void
debug_print_cell_initialisation_options(CellInitialisationOptions *cell_initialisation_options)
{
  const char *cell_initialisation_types[] = {"RANDOM"};
  print("cell_initialisation_type: %s\n", cell_initialisation_types[(u32)cell_initialisation_options->type]);

  print("set_of_initial_states_size: %u\n", cell_initialisation_options->set_of_initial_states_size);

  print("initial_states: ", cell_initialisation_options->set_of_initial_states_size);
  for (u32 i = 0;
       i < cell_initialisation_options->set_of_initial_states_size;
       ++i)
  {
    print(" %d", cell_initialisation_options->set_of_initial_states[i]);
  }
  print("\n");
}


/// Loads a CellInitialisationOptions object from a .cell file.

/// @param[in] file_string  String containing the contents of the .cell file
/// @param[out] cell_intialisation_options  CellInitialisationOptions object to fill in
b32
load_cell_initialisation_options(String file_string, CellInitialisationOptions *cell_initialisation_options, NamedStates *named_states)
{
  b32 success = true;

  String initial_states_string = {};
  b32 initial_states_defined = find_label_value(file_string, "initial_states", &initial_states_string);

  String initialisation_type_string = {};
  b32 initialisation_type_defined = find_label_value(file_string, "initialisation_type", &initialisation_type_string);

  if (initial_states_defined && initialisation_type_defined)
  {
    success &= read_cell_initialisation_type_value(initialisation_type_string, &cell_initialisation_options->type);

    if (success)
    {
      cell_initialisation_options->set_of_initial_states_size = read_named_states_list(named_states, initial_states_string, &cell_initialisation_options->set_of_initial_states);

      if (cell_initialisation_options->set_of_initial_states_size == 0)
      {
        print("No initial states found\n");
        success &= false;
      }
    }
  }

  if (success)
  {
    debug_print_cell_initialisation_options(cell_initialisation_options);
  }
  else
  {
    print("Error whilst loading cell initialisation options.\n");
  }

  return success;
}


/// Loads the .cells file into the various structs.
///
/// Fills: Universe, CellInitialisationOptions, and SimulateOptions
/// UniverseUI holds the .cell file state
/// NamedStates is needed to read the states in the .cells file
///
/// TODO: Why is this in here?
b32
load_universe(const char *filename, Universe *universe, SimulateOptions *simulate_options, CellInitialisationOptions *cell_initialisation_options, NamedStates *named_states)
{
  b32 success = true;

  print("\nLoading universe file: %s\n", filename);

  destroy_cell_hashmap(universe);
  init_cell_hashmap(universe);

  File universe_file;
  String universe_file_string = get_file_string(filename, &universe_file);
  if (universe_file_string.start == 0)
  {
    success &= false;
  }
  else
  {
    success &= load_universe_from_file(universe_file_string, universe, named_states);
    success &= load_simulate_options(universe_file_string, simulate_options);
    success &= load_cell_initialisation_options(universe_file_string, cell_initialisation_options, named_states);

    close_file(&universe_file);
  }

  print("\n");

  return success;
}
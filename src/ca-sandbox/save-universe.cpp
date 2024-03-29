#include "ca-sandbox/save-universe.h"

#include "engine/print.h"
#include "engine/text.h"

#include "ca-sandbox/cell-blocks.h"
#include "ca-sandbox/simulate.h"
#include "ca-sandbox/named-states.h"

#include <stdio.h>

/// @file
/// @brief  Functions for saving a Universe object and all of its CellBlock%s to a .cells file
///


void
serialise_simulate_options(FILE *file_stream, SimulateOptions *simulate_options)
{
  const char *border_type;
  switch (simulate_options->border.type)
  {
    case (BorderType::FIXED):
      border_type = "FIXED";
      break;
    case (BorderType::INFINITE):
      border_type = "INFINITE";
      break;
    case (BorderType::TORUS):
      border_type = "TORUS";
      break;
  }
  fprintf(file_stream, "border_type: %s\n", border_type);

  fprintf(file_stream, "border_min_block: %d %d\n", simulate_options->border.min_corner_block.x, simulate_options->border.min_corner_block.y);
  fprintf(file_stream, "border_min_cell: %d %d\n", simulate_options->border.min_corner_cell.x, simulate_options->border.min_corner_cell.y);
  fprintf(file_stream, "border_max_block: %d %d\n", simulate_options->border.max_corner_block.x, simulate_options->border.max_corner_block.y);
  fprintf(file_stream, "border_max_cell: %d %d\n", simulate_options->border.max_corner_cell.x, simulate_options->border.max_corner_cell.y);

  fprintf(file_stream, "\n");
}


void
serialise_cell_initialisation_options(FILE *file_stream, CellInitialisationOptions *cell_initialisation_options, NamedStates *named_states)
{
  const char *initialisation_type_string = CELL_INITIALISATION_TYPE_STRINGS[(u32)cell_initialisation_options->type];
  fprintf(file_stream, "initialisation_type: %s\n", initialisation_type_string);

  fprintf(file_stream, "initial_states: ");
  for (u32 initial_state_n = 0;
       initial_state_n < cell_initialisation_options->set_of_initial_states.n_elements;
       ++initial_state_n)
  {
    b32 last_state = initial_state_n == cell_initialisation_options->set_of_initial_states.n_elements - 1;
    CellState initial_state = cell_initialisation_options->set_of_initial_states[initial_state_n];
    String initial_state_name = get_state_name(named_states, initial_state);
    fprintf(file_stream, "%.*s%s", string_length(initial_state_name), initial_state_name.start, last_state ? "" : " ");
  }
  fprintf(file_stream, "\n\n");
}


b32
save_universe_to_file(const char *filename, Universe *universe, SimulateOptions *simulate_options, CellInitialisationOptions *cell_initialisation_options, NamedStates *named_states)
{
  b32 success = true;

  FILE *file_stream = fopen(filename, "w");
  if (file_stream == NULL)
  {
    print("Error: Failed to open file %s", filename);
    success &= false;
  }
  else
  {
    fprintf(file_stream, "cell_block_dim: %d\n\n", universe->cell_block_dim);
    serialise_simulate_options(file_stream, simulate_options);
    serialise_cell_initialisation_options(file_stream, cell_initialisation_options, named_states);

    u32 max_state_length = get_longest_state_name_length(named_states);

    u32 cell_block_n = 0;
    for (u32 hash_slot = 0;
         hash_slot < universe->hashmap_size;
         ++hash_slot)
    {
      CellBlock *cell_block = universe->hashmap[hash_slot];

      while (cell_block != 0)
      {
        fprintf(file_stream, "CellBlock: %d, %d\n", cell_block->block_position.x, cell_block->block_position.y);

        for (u32 cell_y = 0;
             cell_y < universe->cell_block_dim;
             ++cell_y)
        {
          for (u32 cell_x = 0;
               cell_x < universe->cell_block_dim;
               ++cell_x)
          {
            u32 cell_index = get_cell_index_in_block(universe, (s32vec2){(s32)cell_x, (s32)cell_y});
            CellState cell_state = cell_block->cell_states[cell_index];

            u32 padding = 0;
            if (cell_x != universe->cell_block_dim - 1)
            {
              padding = max_state_length + 2;
            }

            String cell_state_string = get_state_name(named_states, cell_state);
            fprintf(file_stream, "%-*.*s", padding, string_length(cell_state_string), cell_state_string.start);
          }

          fprintf(file_stream, "\n");
        }
        fprintf(file_stream, "\n");

        ++cell_block_n;

        // Follow any hashmap collision chains
        cell_block = cell_block->next_block;
      }
    }

    fprintf(file_stream, "n_cell_blocks: %d\n", cell_block_n);

    fclose(file_stream);
  }

  return success;
}
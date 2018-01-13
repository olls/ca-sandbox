#include "save-universe.h"

#include "print.h"
#include "text.h"
#include "assert.h"
#include "files.h"
#include "maths.h"

#include "universe.h"
#include "simulate.h"
#include "named-states.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/// @file
/// @brief  Functions for saving a Universe object and all of its CellBlock%s to a .cells file
///

// TODO: Update the file format
// TODO: Fix long state names not working


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


b32
save_universe_to_file(const char *filename, Universe *universe, SimulateOptions *simulate_options, NamedStates *named_states)
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

    // Find longest state name, so we know how far to pad states.
    u32 max_state_length = 0;
    for (u32 state_n = 0;
         state_n < named_states->states.n_elements;
         ++state_n)
    {
      NamedState *named_state = named_states->states.get(state_n);
      max_state_length = max(max_state_length, string_length(named_state->name));
    }

    u32 cell_block_n = 0;
    for (u32 hash_slot = 0;
         hash_slot < universe->hashmap_size;
         ++hash_slot)
    {
      CellBlock *cell_block = universe->hashmap[hash_slot];

      if (cell_block != 0 &&
          cell_block->slot_in_use)
      {
        do
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
              Cell *cell = get_cell_from_block(universe, cell_block, (s32vec2){(s32)cell_x, (s32)cell_y});

              String cell_state = get_state_name(named_states, cell->state);
              fprintf(file_stream, "%-*.*s  ", max_state_length, string_length(cell_state), cell_state.start);
            }

            fprintf(file_stream, "\n");
          }
          fprintf(file_stream, "\n");

          ++cell_block_n;

          // Follow any hashmap collision chains
          cell_block = cell_block->next_block;
        }
        while (cell_block != 0);
      }
    }

    fprintf(file_stream, "n_cell_blocks: %d\n", cell_block_n);

    fclose(file_stream);
  }

  return success;
}
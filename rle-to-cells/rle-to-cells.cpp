#include "types.h"
#include "print.h"
#include "text.h"
#include "files.h"
#include "vectors.h"
#include "cell-storage.h"
#include "save-universe.h"


int
main(int argc, char const *argv[])
{
  b32 success = true;

  if (argc < 3)
  {
    success = false;
  }
  else
  {
    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    File input_file;
    if (!open_file(input_filename, &input_file))
    {
      success = false;
    }
    else
    {
      String file_string = {
        .start = input_file.read_ptr,
        .current_position = input_file.read_ptr,
        .end = input_file.read_ptr + input_file.size
      };

      Universe universe;
      init_cell_hashmap(&universe);
      universe.cell_block_dim = 4;

      CellState default_state = 0;
      CellInitialisationOptions cell_init_opts = {
        .type = CellInitialisationType::RANDOM,
        .set_of_initial_states = &default_state,
        .set_of_initial_states_size = 1
      };

      s32vec2 universe_position = {0, 0};

      b32 at_file_end = false;
      while (!at_file_end)
      {
        String line = get_line(&file_string);

        if (file_string.current_position == file_string.end)
        {
          at_file_end = true;
        }

        if (line.start[0] != '#' &&
            line.start[0] != 'x')
        {
          line.current_position = line.start;
          while (line.current_position < line.end)
          {
            if (line.current_position[0] == '$')
            {
              ++universe_position.y;
              universe_position.x = 0;
            }
            else if (line.current_position[0] == '.')
            {
              u32 state = 0;

              s32vec2 block_position = vec2_divide(universe_position, universe.cell_block_dim);

              CellBlock *cell_block = get_or_create_cell_block(&universe, &cell_init_opts, block_position);

              Cell *cell = get_cell_from_block(&universe, cell_block, vec2_subtract(universe_position, vec2_multiply(block_position, universe.cell_block_dim)));
              cell->state = state;

              ++universe_position.x;
            }
            else if (is_upper_case_letter(line.current_position[0]) && line.current_position[0] <= 'X')
            {
              u32 state = line.current_position[0] - ('A' - 1);

              s32vec2 block_position = vec2_divide(universe_position, universe.cell_block_dim);

              CellBlock *cell_block = get_or_create_cell_block(&universe, &cell_init_opts, block_position);

              Cell *cell = get_cell_from_block(&universe, cell_block, vec2_subtract(universe_position, vec2_multiply(block_position, universe.cell_block_dim)));
              cell->state = state;

              ++universe_position.x;
            }
            else if (is_lower_case_letter(line.current_position[0]) &&
                     ('p' <= line.current_position[0] && line.current_position[0] <= 'y') &&
                     ('A' <= line.current_position[1] && line.current_position[1] <= 'X'))
            {
              u32 state = 24 * (line.current_position[0] - ('p' - 1));
              state += line.current_position[1] - ('A' - 1);

              s32vec2 block_position = vec2_divide(universe_position, universe.cell_block_dim);

              CellBlock *cell_block = get_or_create_cell_block(&universe, &cell_init_opts, block_position);

              Cell *cell = get_cell_from_block(&universe, cell_block, vec2_subtract(universe_position, vec2_multiply(block_position, universe.cell_block_dim)));
              cell->state = state;

              ++universe_position.x;

              ++line.current_position;
            }
            else if (is_num(line.current_position[0]))
            {
              u32 repetitions = get_u32(&line);
              u32 state_to_repeat;

              if (line.current_position[0] == '.')
              {
                state_to_repeat = 0;
              }
              else if (is_upper_case_letter(line.current_position[0]) && line.current_position[0] <= 'X')
              {
                state_to_repeat = line.current_position[0] - ('A' - 1);
              }
              else if (is_lower_case_letter(line.current_position[0]) &&
                       ('p' <= line.current_position[0] && line.current_position[0] <= 'y') &&
                       ('A' <= line.current_position[1] && line.current_position[1] <= 'X'))
              {
                state_to_repeat = 24 * (line.current_position[0] - ('p' - 1));
                state_to_repeat += line.current_position[1] - ('A' - 1);

                ++line.current_position;
              }

              for (u32 i = 0;
                   i < repetitions;
                   ++i)
              {

                s32vec2 block_position = vec2_divide(universe_position, universe.cell_block_dim);

                CellBlock *cell_block = get_or_create_cell_block(&universe, &cell_init_opts, block_position);

                Cell *cell = get_cell_from_block(&universe, cell_block, vec2_subtract(universe_position, vec2_multiply(block_position, universe.cell_block_dim)));
                cell->state = state_to_repeat;

                ++universe_position.x;
              }
            }

            ++line.current_position;
          }
        }

        consume_while(&file_string, is_newline);
      }

      save_universe_to_file(output_filename, &universe);

    }
  }

  return 0;
}
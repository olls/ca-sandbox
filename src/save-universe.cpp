#include  <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "save-universe.h"

#include "print.h"
#include "text.h"
#include "assert.h"
#include "files.h"
#include "cell-storage.h"


void
write_text(WriteString *writer, const char *text, ...)
{
  char buf[1024];
  va_list aptr;
  va_start(aptr, text);
  vsnprintf(buf, 1024, text, aptr);
  va_end(aptr);

  u32 length = strlen(buf);
  assert(writer->current_position + length < writer->end);

  memcpy(writer->current_position, buf, length);

  writer->current_position += length;
}


b32
save_universe_to_file(const char *filename, Universe *universe)
{
  b32 success = true;

  u32 n_cell_blocks = 0;

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
        ++n_cell_blocks;

        // Follow any hashmap collision chains
        cell_block = cell_block->next_block;
      }
      while (cell_block != 0);
    }
  }

  print("n_cell_blocks: %d\n", n_cell_blocks);

  u32 max_bytes_per_cell = 3;

  // Pad all lines to same length to keep positioning simple.
  u32 file_width = max_bytes_per_cell * universe->cell_block_dim;

  u32 n_cell_block_extra_lines = 2; // label + blank line
  u32 n_cell_block_lines = universe->cell_block_dim + n_cell_block_extra_lines;

  u32 n_header_lines = 3; // n_cell_blocks, cell_block_dim, blank line

  u32 estimated_file_lines = n_cell_blocks * (n_cell_block_lines + n_header_lines);
  u32 estimated_file_size = estimated_file_lines * file_width;

  print("estimated_file_size: %d\n", estimated_file_size);

  File file;
  if (!open_file(filename, &file, true, estimated_file_size))
  {
    success = false;
  }
  else
  {
    WriteString file_writer = {
      .start = file.write_ptr,
      .current_position = file.write_ptr,
      .end = file.write_ptr + file.size
    };

    write_text(&file_writer, "n_cell_blocks: %d\n", n_cell_blocks);
    write_text(&file_writer, "cell_block_dim: %d\n\n", universe->cell_block_dim);

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
          write_text(&file_writer, "CellBlock: %d, %d\n", cell_block->block_position.x, cell_block->block_position.y);

          for (u32 cell_y = 0;
               cell_y < universe->cell_block_dim;
               ++cell_y)
          {
            for (u32 cell_x = 0;
                 cell_x < universe->cell_block_dim;
                 ++cell_x)
            {
              Cell *cell = get_cell_from_block(universe, cell_block, (s32vec2){(s32)cell_x, (s32)cell_y});

              write_text(&file_writer, "%d ", cell->state);
            }

            write_text(&file_writer, "\n");
          }
          write_text(&file_writer, "\n");

          ++cell_block_n;

          // Follow any hashmap collision chains
          cell_block = cell_block->next_block;
        }
        while (cell_block != 0);
      }
    }

    close_file(&file, file_writer.current_position - file_writer.start);
  }

  return success;
}
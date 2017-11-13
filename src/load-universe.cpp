#include "load-universe.h"

#include "util.h"
#include "print.h"
#include "text.h"
#include "files.h"
#include "allocate.h"
#include "cell-storage.h"


b32
is_label_char(char character)
{
  b32 result = (is_letter(character) ||
                character == '_');
  return result;
}


b32
label_equals(String label, const char *search)
{
  // Zero length label always returns false (Do we want two zero length strings to match?)
  b32 result = false;

  for (const char *c = label.start;
       c < label.end;
       ++c, ++search)
  {
    if (search == 0 || *c != *search)
    {
      result = false;
      break;
    }
    else
    {
      result = true;
    }
  }

  return result;
}


/// Returns String marking the next line in string, advances string.current_position to end of line.
String
get_line(String *string)
{
  String result;

  // Find the end of the line
  result.start = string->current_position;
  result.current_position = string->current_position;

  consume_until(string, is_newline);
  result.end = string->current_position;

  return result;
}


b32
find_label(String file_string, const char *search_label, s32 *result)
{
  b32 success = true;

  String line;
  String label;

  b32 found_label = false;
  while (!found_label)
  {
    line = get_line(&file_string);

    // Read label

    consume_until(&line, is_letter);
    label.start = line.current_position;

    consume_while(&line, is_label_char);
    label.end = line.current_position;

    if (label_equals(label, search_label))
    {
      found_label = true;
    }

    if (file_string.current_position == file_string.end)
    {
      success = false;
      break;
    }
    else
    {
      // Move past \n character.
      consume_while(&file_string, is_newline);
    }
  }

  if (found_label)
  {
    consume_until_char(&line, ':');
    if (line.current_position == line.end)
    {
      success = false;
      return success;
    }

    ++line.current_position;
    consume_until(&line, is_num_or_sign);

    if (line.current_position == line.end)
    {
      success = false;
      return success;
    }

    const char *num_start = line.current_position;

    *result = get_s32(&line);

    if (num_start == line.current_position)
    {
      success = false;
    }
  }

  return success;
}


void
read_cell_block(String *file_string, Universe *universe)
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

    if (label_equals(label, "CellBlock"))
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

    file_string->current_position = line.end;

    CellBlock *cell_block = create_uninitialised_cell_block(universe, (s32vec2){x, y});

    for (u32 cell_index = 0;
         cell_index < universe->cell_block_dim * universe->cell_block_dim;
         ++cell_index)
    {
      Cell *cell = cell_block->cells + cell_index;

      consume_until(file_string, is_num);

      cell->state = get_u32(file_string);
    }
  }
}


b32
load_universe_from_file(const char filename[], Universe *universe)
{
  b32 success = true;

  print("Loading universe from file: %s\n", filename);

  File file;
  if (!open_file(filename, &file))
  {
    print("Failed to open file: %s\n", filename);
    success = false;
    return success;
  }

  String file_string = {
    .start = file.read_ptr,
    .current_position = file.read_ptr,
    .end = file.read_ptr + file.size
  };

  s32 cell_block_dim;
  s32 n_cell_blocks;

  success &= find_label(file_string, "cell_block_dim", &cell_block_dim);
  success &= find_label(file_string, "n_cell_blocks", &n_cell_blocks);

  if (success)
  {
    print("cell_block_dim: %d\n", cell_block_dim);
    print("n_cell_blocks: %d\n", n_cell_blocks);

    universe->cell_block_dim = cell_block_dim;

    for (u32 cell_block_index = 0;
         cell_block_index < n_cell_blocks;
         ++cell_block_index)
    {
      read_cell_block(&file_string, universe);
    }
  }

  return success;
}
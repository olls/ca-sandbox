#include "parsing.h"

#include "types.h"
#include "allocate.h"
#include "text.h"

#include <string.h>
#include <stdlib.h>

/// @file
/// @brief  Generic functions for parsing `name: value` style text.
///


u32
read_u32_list(String string, u32 **list_result)
{
  u32 n_items = 0;
  u32 list_size = 0;
  *list_result = 0;

  while (string.current_position != string.end)
  {
    consume_until(&string, is_num);

    if (string.current_position != string.end)
    {
      u32 item = get_u32(&string);

      u32 new_n_items = n_items + 1;

      if (new_n_items > list_size)
      {
        if (list_size == 0)
        {
          list_size = 4;
        }
        else
        {
          list_size *= 2;
        }

        u32 *new_list = allocate(u32, list_size);

        if (*list_result != 0)
        {
          memcpy(*list_result, new_list, n_items);
          free(*list_result);
        }

        *list_result = new_list;
      }

      (*list_result)[n_items] = item;
      n_items = new_n_items;
    }
  }

  return n_items;
}


b32
is_label_char(char character)
{
  b32 result = (is_letter(character) ||
                character == '_');
  return result;
}


/// Searches the string for a label identifier, and returns its value
///
/// A label is formatted as `label_name: label value\n`
///
/// This function puts the bounds of the label value into the value_result string. (Trimming white
///   space from the start of the value)
b32
find_label_value(String block, const char *search_label, String *value_result)
{
  b32 success = true;

  String line;
  String label;

  b32 found_label = false;
  while (!found_label)
  {
    line = get_line(&block);

    // Read label

    consume_until(&line, is_letter);
    label.start = line.current_position;

    consume_while(&line, is_label_char);
    label.end = line.current_position;

    if (string_equals(label, search_label))
    {
      found_label = true;
    }
    else
    {
      if (block.current_position == block.end)
      {
        success = false;
        break;
      }
      else
      {
        // Move past \n character.
        consume_while(&block, is_newline);
      }
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
    consume_while(&line, is_whitespace);

    value_result->start = line.current_position;
    value_result->current_position = line.current_position;
    value_result->end = line.end;
  }

  return success;
}


/// Convenience function for finding a label, then getting the u32 value from it's value string.
b32
find_label_value_u32(String file_string, const char *search_label, u32 *result)
{
  b32 success = true;

  String value_string = {};
  b32 label_found = find_label_value(file_string, search_label, &value_string);
  if (label_found)
  {
    *result = get_u32(&value_string);
  }
  else
  {
    success = false;
  }

  return success;
}
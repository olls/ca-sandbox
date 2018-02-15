#include "engine/text.h"

#include "engine/util.h"
#include "engine/types.h"
#include "engine/assert.h"
#include "engine/maths.h"
#include "engine/vectors.h"
#include "engine/files.h"
#include "engine/allocate.h"

#include <string.h>
#include <cstdarg>
#include <stdio.h>

/// @file
/// @brief  Text utility functions and structs
///


/// Creates a String struct pointing at the c-string passed in.
///
/// NOTE: The c-string is not copied, so be careful with scope!
String
new_string(const char *c_string)
{
  String result;

  u32 length = strlen(c_string);

  result.start = c_string;
  result.current_position = result.start;
  result.end = result.start + length;

  return result;
}


/// Creates a String pointing to memory on the stack, using a format string + args
///
/// This is very prone to memory leaks (i.e: user not likely to call free(result.start) )
///
String
new_string_fmt(const char *c_string_format, ...)
{
  String result = {};

  va_list args;
  va_start(args, c_string_format);

  char buffer[512];
  s32 length = vsnprintf(buffer, array_count(buffer), c_string_format, args);
  assert(length >= 0);
  va_end(args);

  // vnsprint_f returns number of bytes which would have been written __without the buffer size
  //   limit__.  Also we need to add one for the NULL byte, which vnsprint_f writes, but doesn't
  //   include in the count.
  length = min(length, (s32)array_count(buffer)) + 1;

  char *new_buffer = allocate(char, length);
  copy_string(new_buffer, buffer, length);

  result.start = new_buffer;
  result.end = result.start + length;

  return result;
}


String
get_file_string(const char *filename, File *file)
{
  String result;

  if (!open_file(filename, file))
  {
    result = {};
  }
  else
  {
    result = {
      .start = file->read_ptr,
      .current_position = file->read_ptr,
      .end = file->read_ptr + file->size
    };
  }

  return result;
}


char *
copy_string(char *dest, const char *source, u32 size)
{
  return strncpy(dest, source, size);
}


void
append_string(WriteString *dest, String source)
{
  u32 copy_length = min(string_length(source), (u32)(dest->end - dest->current_position));
  copy_string(dest->current_position, source.start, copy_length);
  dest->current_position += copy_length;
}


void
append_string(Array::Array<char>& dynamic_string, String source)
{
  Array::add_n(dynamic_string, (char *)source.start, string_length(source));
}


char *
dynamic_string_to_heap(Array::Array<char>& dynamic_string)
{
  char *result = allocate(char, dynamic_string.n_elements + 1);
  copy_string(result, dynamic_string.elements, dynamic_string.n_elements);
  result[dynamic_string.n_elements] = '\0';
  return result;
}


b32
str_eq(const char *a, const char *b, u32 n)
{
  for (u32 i = 0; i < n; ++i)
  {
    if (a[i] != b[i])
    {
      return false;;
    }
  }

  return true;
}


b32
is_num(char character)
{
  b32 result = (character >= '0') && (character <= '9');
  return result;
}


b32
is_num_or_sign(char character)
{
  b32 result = is_num(character) || character == '-' || character == '+';
  return result;
}


b32
is_comparison_op(char character)
{
  b32 result = (character == '>' ||
                character == '<' ||
                character == '=');
  return result;
}


b32
is_letter(char character)
{
  b32 result = (((character >= 'a') && (character <= 'z')) ||
                ((character >= 'A') && (character <= 'Z')));
  return result;
}


b32
is_lower_case_letter(char character)
{
  b32 result = ((character >= 'a') && (character <= 'z'));
  return result;
}


b32
is_upper_case_letter(char character)
{
  b32 result = ((character >= 'A') && (character <= 'Z'));
  return result;
}


b32
is_newline(char character)
{
  b32 result = (character == '\n') || (character == '\r');
  return result;
}


b32
is_whitespace(char character)
{
  b32 result = (character == ' ') || (character == '\t');
  return result;
}


b32
is_whitespace_or_nl(char character)
{
  b32 result = is_whitespace(character) || is_newline(character);
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


/// Returns true if string is a prefix of search
///
/// Zero length string always returns false (Do we want two zero length strings to match?)
b32
string_equals(String string, const char *search)
{
  b32 result = false;

  for (const char *c = string.start;
       c < string.end;
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


b32
strings_equal(String *a, String *b)
{
  b32 success = false;

  u32 a_length = a->end - a->start;
  u32 b_length = b->end - b->start;

  if (a_length == b_length)
  {
    success = true;

    for (u32 char_index = 0;
         char_index < a_length;
         ++char_index)
    {
      if (a->start[char_index] != b->start[char_index])
      {
        success = false;
        break;
      }
    }
  }

  return success;
}


u32
string_length(String string)
{
  u32 result = string.end - string.start;
  return result;
}


u32
string_length(WriteString string)
{
  u32 result = string.end - string.start;
  return result;
}


u32
get_u32(String *string)
{
  const char *num_start = string->current_position;

  consume_while(string, is_num);

  u32 result = 0;
  u32 num_length = string->current_position - num_start;

  for (u32 num_pos = 0;
       num_pos < num_length;
       ++num_pos)
  {
    u32 digit = *(num_start + num_pos) - '0';
    result += digit * pow(10, (num_length - num_pos - 1));
  }

  return result;
}


s32
get_s32(String *string)
{
  s32 result = 0;

  b32 coef = 1;
  if (string->current_position != string->end)
  {
    if (*string->current_position == '-')
    {
      ++string->current_position;
      coef = -1;
    }
    else if (*string->current_position == '+')
    {
      ++string->current_position;
      coef = 1;
    }
  }

  const char *num_start = string->current_position;

  consume_while(string, is_num);

  u32 num_length = string->current_position - num_start;

  for (u32 num_pos = 0;
       num_pos < num_length;
       ++num_pos)
  {
    u32 digit = *(num_start + num_pos) - '0';
    result += digit * pow(10, (num_length - num_pos - 1));
  }

  result *= coef;

  return result;
}


r32
get_r32(String *string)
{
  s32 whole_num = get_s32(string);

  r32 result = whole_num;

  if (string->current_position != string->end && *string->current_position == '.')
  {
    ++string->current_position;

    u32 frac_num = get_u32(string);

    r32 frac_part = frac_num;
    while (frac_part >= 1)
    {
      frac_part /= 10;
    }

    if (whole_num < 0)
    {
      frac_part *= -1;
    }

    result += frac_part;
  }

  return result;
}


b32
get_vector(String string, s32vec2 *result)
{
  b32 success = true;

  consume_until(&string, is_num_or_sign);

  if (string.current_position == string.end)
  {
    success = false;
  }
  else
  {
    const char *num_start = string.current_position;

    result->x = get_s32(&string);

    if (num_start == string.current_position)
    {
      success = false;
    }
  }

  if (success)
  {
    consume_until(&string, is_num_or_sign);

    if (string.current_position == string.end)
    {
      success = false;
    }
    else
    {
      const char *num_start = string.current_position;

      result->y = get_s32(&string);

      if (num_start == string.current_position)
      {
        success = false;
      }
    }
  }

  return success;
}
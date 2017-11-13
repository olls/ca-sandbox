#include <string.h>

#include "text.h"

#include "types.h"
#include "maths.h"


char *
copy_string(char *dest, const char *source, u32 size)
{
  return (char *)strncpy((char *)dest, (const char *)source, size);
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
is_letter(char character)
{
  b32 result = (((character >= 'a') && (character <= 'z')) ||
                ((character >= 'A') && (character <= 'Z')));
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
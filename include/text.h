#ifndef TEXT_H_DEF
#define TEXT_H_DEF

#include "types.h"
#include "vectors.h"
#include "files.h"

/// @file
/// @brief  Text utilities
///


/// A simple struct for keeping track of the end point of a string, and your current position within
///   the string.
struct String
{
  const char *start;
  const char *current_position;
  const char *end;
};


/// Same as String, but without const
struct WriteString
{
  char *start;
  char *current_position;
  char *end;
};


#define consume_while(string, func) while ((string)->current_position < (string)->end && func(*((string)->current_position))) ++((string)->current_position)
#define consume_until(string, func) while ((string)->current_position < (string)->end && !func(*((string)->current_position))) ++((string)->current_position)
#define consume_until_char(string, c) while ((string)->current_position < (string)->end && *((string)->current_position) != (c)) ++((string)->current_position)


String
new_string(const char *c_string);


String
get_file_string(const char *filename, File *file);


char *
copy_string(char *dest, const char *source, u32 size);


void
append_string(WriteString *dest, String source);


b32
str_eq(const char *a, const char *b, u32 n);


b32
is_num(char character);


b32
is_num_or_sign(char character);


b32
is_comparison_op(char character);


b32
is_letter(char character);


b32
is_lower_case_letter(char character);


b32
is_upper_case_letter(char character);


b32
is_newline(char character);


b32
is_whitespace(char character);


b32
is_whitespace_or_nl(char character);


String
get_line(String *string);


b32
string_equals(String string, const char *search);


b32
strings_equal(String *string, String *search);


u32
string_length(String string);


u32
string_length(WriteString string);


u32
get_u32(String *string);


s32
get_s32(String *string);


r32
get_r32(String *string);


b32
get_vector(String string, s32vec2 *result);


#endif

#ifndef PARSING_H_DEF
#define PARSING_H_DEF

#include "types.h"
#include "text.h"

/// @file
///


u32
read_u32_list(String string, u32 **list_result);


b32
find_label_value(String file_string, const char *search_label, String *value_result);


b32
is_label_char(char character);


b32
find_label_value_u32(String file_string, const char *search_label, u32 *result);


#endif
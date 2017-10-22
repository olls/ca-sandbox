#ifndef OPENGL_UTIL_H_DEF
#define OPENGL_UTIL_H_DEF

#include "types.h"


#define opengl_print_errors() _opengl_print_errors(__FILE__, __LINE__)

b32
_opengl_print_errors(const char *file, u32 line);


#endif
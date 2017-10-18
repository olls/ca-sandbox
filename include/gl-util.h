#ifndef GL_UTIL_H_DEF
#define GL_UTIL_H_DEF

#include "types.h"

#define gl_print_errors() _gl_print_errors(__FILE__, __LINE__)

b32
_gl_print_errors(const char *file, u32 line);


#endif
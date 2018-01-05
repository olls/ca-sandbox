#ifndef OPENGL_UTIL_H_DEF
#define OPENGL_UTIL_H_DEF

#include "types.h"

#include <GL/glew.h>

#define opengl_print_errors() _opengl_print_errors(__FILE__, __LINE__)

/// @file
/// @brief  OpenGL utilities
///


b32
_opengl_print_errors(const char *file, u32 line);


void
opengl_debug_output_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, void *user_param);


void
opengl_debug_output_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *user_param);


#endif
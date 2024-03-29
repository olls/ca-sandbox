#ifndef OPENGL_SHADERS_H_DEF
#define OPENGL_SHADERS_H_DEF

#include "types.h"

#include <GL/glew.h>


b32
create_shader_program(const char *filenames[], GLenum types[], u32 n_shaders, GLuint *result);


#endif
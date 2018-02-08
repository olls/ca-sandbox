#ifndef SCREEN_SHADER_H_DEF
#define SCREEN_SHADER_H_DEF

#include "engine/types.h"
#include "engine/opengl-buffer.h"

#include <GL/glew.h>


struct ScreenShader
{
  GLuint vao;

  GLuint shader_program;
  GLuint aspect_ratio_uniform;

  OpenGL_Buffer index_buffer;
  OpenGL_Buffer vertex_buffer;
  OpenGL_Buffer colour_buffer;
};


b32
initialise_screen_shader(ScreenShader *screen_shader);


void
initialise_screen_shader_attributes(ScreenShader *screen_shader);


#endif
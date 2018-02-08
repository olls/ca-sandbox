#include "ca-sandbox/screen-shader.h"

#include "engine/opengl-shaders.h"
#include "engine/opengl-util.h"
#include "engine/opengl-buffer.h"
#include "engine/print.h"
#include "engine/vectors.h"


b32
initialise_screen_shader(ScreenShader *screen_shader)
{
  b32 success = true;

  // Compile shaders

  GLenum shader_types[] = {
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER
  };
  const char *screen_filenames[] = {
    "shaders/screen.glvs",
    "shaders/screen.glfs"
  };
  success &= create_shader_program(screen_filenames, shader_types, 2, &screen_shader->shader_program);

  // Get shader uniforms

  screen_shader->aspect_ratio_uniform = glGetUniformLocation(screen_shader->shader_program, "aspect_ratio");
  if (screen_shader->aspect_ratio_uniform == -1)
  {
    print("Failed to find uniform screen_shader->aspect_ratio_uniform\n");
    success &= false;
  }

  // Create VAO
  glGenVertexArrays(1, &screen_shader->vao);

  // Create buffers

  create_opengl_buffer(&screen_shader->index_buffer, sizeof(GLushort), GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_DRAW);
  create_opengl_buffer(&screen_shader->vertex_buffer, sizeof(vec2), GL_ARRAY_BUFFER, GL_STREAM_DRAW);
  create_opengl_buffer(&screen_shader->colour_buffer, sizeof(vec4), GL_ARRAY_BUFFER, GL_STREAM_DRAW);

  opengl_print_errors();
  return success;
}


void
initialise_screen_shader_attributes(ScreenShader *screen_shader)
{
  glBindBuffer(screen_shader->vertex_buffer.binding_target, screen_shader->vertex_buffer.id);
  GLuint attribute_pos = glGetAttribLocation(screen_shader->shader_program, "pos");
  if (attribute_pos == -1)
  {
    print("Failed to get attribute_pos\n");
  }
  glEnableVertexAttribArray(attribute_pos);
  glVertexAttribPointer(attribute_pos, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 0);

  glBindBuffer(screen_shader->colour_buffer.binding_target, screen_shader->colour_buffer.id);
  GLuint attribute_colour = glGetAttribLocation(screen_shader->shader_program, "colour");
  if (attribute_colour == -1)
  {
    print("Failed to get attribute_colour\n");
  }
  glEnableVertexAttribArray(attribute_colour);
  glVertexAttribPointer(attribute_colour, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), 0);

  opengl_print_errors();
}
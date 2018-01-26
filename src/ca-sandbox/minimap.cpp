#include "ca-sandbox/minimap.h"

#include "ca-sandbox/cell-blocks.h"
#include "ca-sandbox/cell-regions.h"
#include "ca-sandbox/cell-drawing.h"

#include "engine/print.h"
#include "engine/opengl-util.h"
#include "engine/opengl-buffer.h"

#include "imgui/imgui.h"

#include <GL/glew.h>


GLuint
create_minimap_framebuffer()
{
  GLuint result = 0;

  glGenFramebuffers(1, &result);

  return result;
}


GLuint
create_minimap_texture(s32vec2 size, GLuint framebuffer)
{
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  GLuint result = 0;
  glGenTextures(1, &result);
  glBindTexture(GL_TEXTURE_2D, result);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, result, 0);

  const u32 n_draw_buffers = 1;
  GLenum draw_buffers[n_draw_buffers] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(n_draw_buffers, draw_buffers);

  opengl_print_errors();
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    result = 0;
    print("Error whist initialising minimap framebuffer texture\n");
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  return result;
}


void
draw_minimap_texture(CellBlocks *cell_blocks, CellInstancing *cell_instancing, CellDrawing *cell_drawing, OpenGL_Buffer *general_vertex_buffer, GLuint framebuffer, GLuint texture, s32vec2 texture_size)
{
  mat4x4 mini_map_projection_matrix;
  mat4x4Identity(mini_map_projection_matrix);

  s32vec2 lowest_coord;
  s32vec2 highest_coord;
  get_cell_blocks_dimentions(cell_blocks, &lowest_coord, &highest_coord);

  vec3 offset = {};
  offset.xy = vec2_multiply(s32vec2_to_vec2(vec2_add(vec2_add(lowest_coord, highest_coord), 1)), -0.5);
  mat4x4Translate(mini_map_projection_matrix, offset);

  mat4x4 aspect_ratio;
  mat4x4Identity(aspect_ratio);
  aspect_ratio[0][0] = (r32)texture_size.y / texture_size.x;

  mat4x4 mini_map_projection_matrix_aspect;
  mat4x4MultiplyMatrix(mini_map_projection_matrix_aspect, mini_map_projection_matrix, aspect_ratio);

  s32vec2 dim = vec2_add(vec2_subtract(highest_coord, lowest_coord), 1);
  vec2 dim_ratio_to_texture = vec2_divide(s32vec2_to_vec2(dim), s32vec2_to_vec2(texture_size));
  r32 scale = min(((r32)texture_size.x / texture_size.y)*(2.0/dim.x), 2.0/dim.y);
  mat4x4Scale(mini_map_projection_matrix_aspect, scale);

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glBindTexture(GL_TEXTURE_2D, texture);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

  const u32 n_draw_buffers = 1;
  GLenum draw_buffers[n_draw_buffers] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(n_draw_buffers, draw_buffers);

  glViewport(0, 0, texture_size.x, texture_size.y);
  glClearColor(1, 1, 1, 0);
  glClear(GL_COLOR_BUFFER_BIT);

  draw_cell_blocks(cell_blocks, cell_instancing, cell_drawing, general_vertex_buffer, mini_map_projection_matrix_aspect);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  opengl_print_errors();
}


void
draw_minimap_texture_to_screen(GLuint framebuffer, GLuint texture, s32vec2 texture_size, GLuint texture_shader, GLuint rendered_texture_uniform)
{
  // glUseProgram(texture_shader);

  // glActiveTexture(GL_TEXTURE0);
  // glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  // glBindTexture(GL_TEXTURE_2D, texture);

  // glUniform1i(rendered_texture_uniform, 0);

  // glViewport(0, 0, texture_size.x, texture_size.y);
  // glBindTexture(GL_TEXTURE_2D, 0);
  // glBindFramebuffer(GL_FRAMEBUFFER, 0);

  ImTextureID tex_id = (void *)(intptr_t)texture;
  ImGui::Image(tex_id, s32vec2_to_vec2(texture_size), ImVec2(0,0), ImVec2(1,1), ImColor(0xFF, 0xFF, 0xFF, 0xFF), ImColor(0xFF, 0xFF, 0xFF, 0x80));

  opengl_print_errors();
}
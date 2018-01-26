#ifndef DRAWING_H_DEF
#define DRAWING_H_DEF

#include "engine/opengl-buffer.h"

#include <string.h>


template <typename PositionType, typename VertexType>
u32
make_square_triangle_vertices(PositionType top_left_corner, PositionType size, VertexType template_vertex, u32 position_type_position_in_template, OpenGL_Buffer *vbo, OpenGL_Buffer *ibo)
{
  PositionType positions[] = {
    vec2_add(top_left_corner, vec2_multiply(size, {0, 0})),
    vec2_add(top_left_corner, vec2_multiply(size, {1, 0})),
    vec2_add(top_left_corner, vec2_multiply(size, {1, 1})),
    vec2_add(top_left_corner, vec2_multiply(size, {0, 1}))
  };

  GLushort vbo_index[4];
  for (u32 i = 0;
       i < 4;
       ++i)
  {
    VertexType vertex = template_vertex;
    memcpy((u8 *)(&vertex) + position_type_position_in_template, positions + i, sizeof(PositionType));
    vbo_index[i] = opengl_buffer_new_element(vbo, &vertex);
  }

  GLuint start_pos = ibo->elements_used;

  opengl_buffer_new_element(ibo, &vbo_index[0]);
  opengl_buffer_new_element(ibo, &vbo_index[1]);
  opengl_buffer_new_element(ibo, &vbo_index[2]);
  opengl_buffer_new_element(ibo, &vbo_index[0]);
  opengl_buffer_new_element(ibo, &vbo_index[2]);
  opengl_buffer_new_element(ibo, &vbo_index[3]);

  GLuint end_pos = ibo->elements_used;

  return end_pos - start_pos;
}


#endif
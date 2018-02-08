#ifndef DRAWING_H_DEF
#define DRAWING_H_DEF

#include "engine/opengl-buffer.h"

#include <string.h>


struct BufferDrawingLocation
{
  u32 start_position;
  u32 n_elements;
};


/// Uploads the four vertices of a square to the provided vbo.
/// Uploads the indices (of those vertices) needed to draw two triangles covering the square to the
///   provided ibo.
///
/// PositionType is the input type of the position and size vectors, and the type of an member of
///   the VertexType
/// VertexType is the output of the vertex to be uploaded to the vbo - containing a VertexType.
/// base_vertex is the default vertex which is uploaded for each vertex
/// position_type_position_in_base_vertex tells the function where in the base_vertex to write the
///   PositionType too in order to create the four vertices of the square.
///
/// n_uploaded_{vertices,indices} are incremented for each vertex/index uploaded.
///
template <typename PositionType, typename VertexType>
u32
upload_square(PositionType start_corner, PositionType size, VertexType base_vertex, u32 position_type_position_in_base_vertex,
              OpenGL_Buffer *vbo, OpenGL_Buffer *ibo, u32 *n_uploaded_vertices, u32 *n_uploaded_indices)
{
  u32 n_vertices = 0;

  PositionType positions[] = {
    vec2_add(start_corner, vec2_multiply(size, {0, 0})),
    vec2_add(start_corner, vec2_multiply(size, {1, 0})),
    vec2_add(start_corner, vec2_multiply(size, {1, 1})),
    vec2_add(start_corner, vec2_multiply(size, {0, 1}))
  };

  GLushort vbo_index[4];
  for (u32 i = 0;
       i < 4;
       ++i)
  {
    VertexType vertex = base_vertex;
    memcpy((u8 *)(&vertex) + position_type_position_in_base_vertex, positions + i, sizeof(PositionType));
    vbo_index[i] = opengl_buffer_new_element(vbo, &vertex);
    ++n_vertices;
  }

  GLuint start_pos = ibo->elements_used;

  opengl_buffer_new_element(ibo, &vbo_index[0]);
  opengl_buffer_new_element(ibo, &vbo_index[1]);
  opengl_buffer_new_element(ibo, &vbo_index[2]);
  opengl_buffer_new_element(ibo, &vbo_index[0]);
  opengl_buffer_new_element(ibo, &vbo_index[2]);
  opengl_buffer_new_element(ibo, &vbo_index[3]);

  GLuint end_pos = ibo->elements_used;
  (*n_uploaded_indices) += end_pos - start_pos;
  (*n_uploaded_vertices) += n_vertices;

  return n_vertices;
}


template <typename VertexType>
u32
upload_square(VertexType start_corner, VertexType size,
              OpenGL_Buffer *vbo, OpenGL_Buffer *ibo, u32 *n_uploaded_vertices, u32 *n_uploaded_indices)
{
  VertexType base_vertex;
  return upload_square(start_corner, size, base_vertex, 0, vbo, ibo, n_uploaded_vertices, n_uploaded_indices);
}


template <typename VertexType>
u32
upload_outline(VertexType start_corner, VertexType end_corner, VertexType outline_size,
              OpenGL_Buffer *vbo, OpenGL_Buffer *ibo, u32 *n_uploaded_vertices, u32 *n_uploaded_indices)
{
  u32 n_vertices = 0;

  VertexType inner_size = vec2_subtract(end_corner, start_corner);
  VertexType outer_size = vec2_add(inner_size, vec2_multiply(outline_size, 2));

  VertexType outer_start = vec2_subtract(start_corner, outline_size);
  VertexType outer_end = vec2_add(end_corner, outline_size);

  // Top
  n_vertices += upload_square((VertexType){outer_start.x,  outer_start.y},
                              (VertexType){outer_size.x,   outline_size.y}, vbo, ibo, n_uploaded_vertices, n_uploaded_indices);

  // Bottom
  n_vertices += upload_square((VertexType){outer_start.x,  end_corner.y},
                              (VertexType){outer_size.x,   outline_size.y}, vbo, ibo, n_uploaded_vertices, n_uploaded_indices);

  // Left
  n_vertices += upload_square((VertexType){outer_start.x,  start_corner.y},
                              (VertexType){outline_size.x, inner_size.y}, vbo, ibo, n_uploaded_vertices, n_uploaded_indices);

  // Right
  n_vertices += upload_square((VertexType){end_corner.x,   start_corner.y},
                              (VertexType){outline_size.x, inner_size.y}, vbo, ibo, n_uploaded_vertices, n_uploaded_indices);

  return n_vertices;
}


#endif
#include "opengl-general-buffers.h"
#include "vectors.h"


/// @file
/// @brief General OpenGL_Buffer%s for vertices and indices to be used for variety of purposes.


/// Creates the two general OpenGL_Buffer%s for vertex and index storage
void
opengl_create_general_buffers(OpenGL_Buffer *general_vertex_buffer,
                              OpenGL_Buffer *general_index_buffer)
{
  create_opengl_buffer(general_vertex_buffer, sizeof(vec2), GL_ARRAY_BUFFER, GL_STATIC_DRAW);
  create_opengl_buffer(general_index_buffer, sizeof(GLushort), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
}
#include "opengl-general-buffers.h"
#include "vectors.h"


void
opengl_create_general_buffers(OpenGL_Buffer *general_vertex_buffer,
                              OpenGL_Buffer *general_index_buffer)
{
  create_opengl_buffer(general_vertex_buffer, sizeof(vec2), GL_ARRAY_BUFFER, GL_STATIC_DRAW);
  create_opengl_buffer(general_index_buffer, sizeof(GLushort), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
}
#ifndef OPENGL_GENERAL_BUFFERS_H_DEF
#define OPENGL_GENERAL_BUFFERS_H_DEF

#include "opengl-buffer.h"

/// @file
///


typedef OpenGL_Buffer GeneralVertexBuffer;
typedef OpenGL_Buffer GeneralIndexBuffer;


void
opengl_create_general_buffers(GeneralVertexBuffer *general_vertex_buffer,
                              GeneralIndexBuffer *general_index_buffer);


#endif
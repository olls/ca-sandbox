#ifndef OPENGL_BUFFERS_H_DEF
#define OPENGL_BUFFERS_H_DEF


// Implements a continuous array in an OpenGL buffer:
// - Allocates a block of memory in OpenGL
// - On element deletion, last element in array is moved into deleted elements position.
//   - This means element indices are not constant, and an element's position in the buffer can not be assumed.
// - On element addition, element is prepended to end of the elements.
// - If buffer reaches capacity, new buffer is allocated with twice the previous capacity, and elements are copied into it.


#include <GL/glew.h>

#include "types.h"
#include "limits.h"


const u32 INITIAL_GL_BUFFER_TOTAL_ELEMENTS = 16;
const u32 MAX_BUFFER_SIZE = MAX_U32;


struct OpenGL_Buffer
{
  GLuint id;

  u32 element_size;
  u32 total_elements;
  u32 elements_used;

  GLenum binding_target;
  GLenum usage;

  void (*setup_attributes_function)(OpenGL_Buffer *);
};


#endif
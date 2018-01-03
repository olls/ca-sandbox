#ifndef OPENGL_BUFFER_H_DEF
#define OPENGL_BUFFER_H_DEF

#include "types.h"

#include <GL/glew.h>


///@file
///
/// Implements a continuous array in an OpenGL buffer object:
/// - Allocates a block of memory in OpenGL
/// - On element deletion, last element in array is moved into deleted elements position.
///   - This means element indices are not constant, and an element's position in the buffer can not be assumed.
/// - On element addition, element is prepended to end of the elements.
/// - If buffer reaches capacity, new buffer is allocated with twice the previous capacity, and elements are copied into it.


/// The initial size for all OpenGL_Buffer%s created with create_opengl_buffer()
const u32 INITIAL_GL_BUFFER_TOTAL_ELEMENTS = 16;

/// The maximum size in bytes of an OpenGL_Buffer
const u32 MAX_BUFFER_SIZE = MAX_U32;


/// Holds state and references for an allocated OpenGL buffer
struct OpenGL_Buffer
{
  GLuint id;  ///< The reference to the OpenGL buffer object

  u32 element_size;  ///< The size of each element in this buffer in bytes
  u32 total_elements;  ///< The total number of elements this buffer can currently hold
  u32 elements_used;  ///< The total number of elements this buffer is currently holding

  GLenum binding_target;  ///< OpenGL binding target to be used
  GLenum usage;  ///< OpenGL usage pattern

  /// @brief The function to be called after re-allocating the buffer, needed for re-assigning the
  ///          attributes.
  void (*setup_attributes_function)(OpenGL_Buffer *);
};


void
create_opengl_buffer(OpenGL_Buffer *buffer, u32 element_size, GLenum binding_target, GLenum usage, u32 size = 0);


void
opengl_buffer_extend(OpenGL_Buffer *buffer, u32 minimum_new_total_elements = 0);


void
opengl_buffer_update_element(OpenGL_Buffer *buffer, u32 element_position, void *new_element);


u32
opengl_buffer_new_element(OpenGL_Buffer *buffer, void *element);


u32
opengl_buffer_add_elements(OpenGL_Buffer *buffer, u32 n_elements, void *elements);


void
opengl_buffer_remove_element(OpenGL_Buffer *buffer, u32 element_to_remove);


void
opengl_buffer_get_element(OpenGL_Buffer *buffer, u32 element_position, void *result);


#endif
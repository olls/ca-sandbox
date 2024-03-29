#include "engine/opengl-buffer.h"

#include "engine/types.h"
#include "engine/print.h"
#include "engine/maths.h"
#include "engine/assert.h"
#include "engine/opengl-util.h"

#include <GL/glew.h>

/// @file
/// @brief Functions for manipulating OpenGL_Buffer objects


/// Generates a new OpenGL buffer object, sets the members of OpenGL_Buffer and allocates the
///   initial size of the buffer in OpenGL.
///
/// @param[out] buffer
/// @param[in] element_size  The size of an element in this buffer in bytes
/// @param[in] binding_target
/// @param[in] usage
/// @param[in] size  The initial number of elements the buffer should be allocated for
///
void
create_opengl_buffer(OpenGL_Buffer *buffer, u32 element_size, GLenum binding_target, GLenum usage, u32 size)
{
  glGenBuffers(1, &buffer->id);

  buffer->element_size = element_size;
  buffer->binding_target = binding_target;
  buffer->usage = usage;
  buffer->elements_used = 0;
  buffer->total_elements = max(size, INITIAL_GL_BUFFER_TOTAL_ELEMENTS);

  // TODO: Can we do without this?
  buffer->setup_attributes_function = 0;

  print("Allocating new OpenGL buffer of size %u\n", buffer->total_elements);

  glBindBuffer(buffer->binding_target, buffer->id);
  glBufferData(buffer->binding_target, buffer->element_size * buffer->total_elements, NULL, GL_STATIC_DRAW);

  opengl_print_errors();
}


/// Re-allocate an OpenGL_Buffer object to hold at least n additional elements.
///
/// Copies the old data into the new buffer, element indices remain intact.
///
void
opengl_buffer_extend(OpenGL_Buffer *buffer, u32 minimum_new_total_elements)
{
  print("Out of space in OpenGL buffer, allocating larger.\n");

  assert(buffer->total_elements != 0);

  // At least one double, then as many more as needed to be greater than minimum_new_total_elements
  u32 new_total_elements = buffer->total_elements * 2;
  do
  {
    u32 current_buffer_size = buffer->element_size * buffer->total_elements;
    assert(current_buffer_size < MAX_BUFFER_SIZE / 2);

    new_total_elements *= 2;
  }
  while (new_total_elements <= minimum_new_total_elements);

  // Create new buffer with size new_total_elements

  GLuint new_buffer;
  glGenBuffers(1, &new_buffer);

  glBindBuffer(buffer->binding_target, new_buffer);
  glBufferData(buffer->binding_target, buffer->element_size * new_total_elements, NULL, GL_STATIC_DRAW);

  // Copy data into new buffer

  // Bind buffers to special READ/WRITE copying buffers
  glBindBuffer(GL_COPY_READ_BUFFER, buffer->id);
  glBindBuffer(GL_COPY_WRITE_BUFFER, new_buffer);

  glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, buffer->element_size * buffer->elements_used);

  glBindBuffer(GL_COPY_READ_BUFFER, 0);
  glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

  // Swap buffers

  glDeleteBuffers(1, &buffer->id);

  buffer->id = new_buffer;
  buffer->total_elements = new_total_elements;

  // Re-setup attributes if needed
  if (buffer->setup_attributes_function != 0)
  {
    buffer->setup_attributes_function(buffer);
  }

  opengl_print_errors();
  print("Reallocated VBO.\n");
}


/// Overwrite an element in an OpenGL_Buffer object
///
/// @param[in] buffer
/// @param[in] element_position  The index of the element within the buffer
/// @param[in] new_element  The value to overwrite with.  Must point to an object with the same size
///                           as `buffer.element_size`.
///
void
opengl_buffer_update_element(OpenGL_Buffer *buffer, u32 element_position, void *new_element)
{
  if (element_position < buffer->elements_used)
  {
    glBindBuffer(buffer->binding_target, buffer->id);
    glBufferSubData(buffer->binding_target, buffer->element_size * element_position, buffer->element_size, new_element);
  }
  else
  {
    print("Cannot update element %u. OpenGL buffer only has %u elements\n", element_position, buffer->elements_used);
  }

  opengl_print_errors();
}


/// Adds a new element to the end of an OpenGL_Buffer, extending it if necessary.
///
/// @param[in] buffer
/// @param[in] element  The element to be added.  Must point to an object with the same size as
///                       `buffer.element_size`.
u32
opengl_buffer_new_element(OpenGL_Buffer *buffer, void *element)
{
  // print("Adding new element to OpenGL buffer.\n");

  if (buffer->elements_used >= buffer->total_elements)
  {
    opengl_buffer_extend(buffer);
  }

  u32 position = buffer->elements_used;
  ++buffer->elements_used;

  opengl_buffer_update_element(buffer, position, element);

  opengl_print_errors();
  return position;
}


/// Add an array of new elements to the end of an OpenGL_Buffer, extending it if necessary.
///
/// @param[in] buffer
/// @param[in] n_elements  The number of elements to be added to the buffer.
/// @param[in] elements  Pointer to array of elements, this array should be
///                        `buffer.element_size * n_elements` bytes big.
///
u32
opengl_buffer_add_elements(OpenGL_Buffer *buffer, u32 n_elements, void *elements)
{
  u32 start_position = -1;

  if (n_elements > 0)
  {
    if (buffer->total_elements - buffer->elements_used < n_elements)
    {
      u32 new_total_elements_needed = n_elements + buffer->elements_used;

      opengl_buffer_extend(buffer, new_total_elements_needed);
    }

    start_position = buffer->elements_used;

    glBindBuffer(buffer->binding_target, buffer->id);
    glBufferSubData(buffer->binding_target, buffer->element_size * start_position, buffer->element_size * n_elements, elements);

    buffer->elements_used += n_elements;
  }

  return start_position;
}


/// Remove an element from an OpenGL_Buffer, moving the last element in the buffer into its position
///   to maintain a contiguous array of elements.  Because of this, as soon as this function is
///   called any element indices which have previously been stored can no longer be assumed to be
///   correct.  Therefore if you need to retain element indicies, you should avoid this function.
///
/// @param[in] buffer
/// @param[in] element_to_remove  The index of the element which is to be removed.
///
void
opengl_buffer_remove_element(OpenGL_Buffer *buffer, u32 element_to_remove)
{
  if (element_to_remove < buffer->elements_used)
  {
    glBindBuffer(buffer->binding_target, buffer->id);

    // Reduce size of buffer by one, this is also now indicating the position of the element we need to move into the removed element's slot.
    --buffer->elements_used;
    u32 element_to_move = buffer->elements_used;

    if (element_to_move == 0 ||
        element_to_remove == element_to_move)
    {
      // All elements have been removed, or the element we were removing was at the end of the array, hence no more action is needed.
    }
    else
    {
      // Copy last element into the removed element's slot
      u32 element_to_remove_position = element_to_remove * buffer->element_size;
      u32 element_to_move_position = element_to_move * buffer->element_size;

      glCopyBufferSubData(buffer->binding_target, buffer->binding_target, element_to_move_position, element_to_remove_position, buffer->element_size);
    }
  }
  opengl_print_errors();
}


/// Downloads an element from an OpenGL_Buffer
///
/// @param[in] buffer
/// @param[in] element_position  The index of the element to retrieve
/// @param[out] result  A pointer to allocated memory to place the result in. Must have a size of
///                       `buffer.element_size`.
///
void
opengl_buffer_get_element(OpenGL_Buffer *buffer, u32 element_position, void *result)
{
  glBindBuffer(buffer->binding_target, buffer->id);
  glGetBufferSubData(buffer->binding_target, buffer->element_size * element_position, buffer->element_size, result);
}

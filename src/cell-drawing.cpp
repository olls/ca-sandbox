#include <GL/glew.h>

#include "util.h"
#include "print.h"
#include "opengl-util.h"
#include "opengl-buffer.h"
#include "opengl-general-buffers.h"
#include "cell-drawing.h"
#include "cell-storage.h"


/// @file
/// @brief Functions for drawing the Universe to the screen using OpenGL instancing.


/// @brief Initialise the CellInstancing struct by creating the OpenGL_Buffer which holds the
///          CellInstances, and upload the cell vertices/indices to the general buffers.
///
/// @param[out] cell_instancing  Pointer to the struct to fill out.
/// @param[in] general_vertex_buffer  Pointer to a general OpenGL_Buffer setup to take `vec2`
///                                     elements.
/// @param[in] general_index_buffer  Pointer to a general OpenGL_Buffer setup to take `GLushort`
///                                    index elements.
///
void
init_cell_drawing(CellInstancing *cell_instancing, OpenGL_Buffer *general_vertex_buffer, OpenGL_Buffer *general_index_buffer)
{
  vec2 vertices[] = {
    {0, 0},
    {0, 1},
    {1, 1},
    {1, 0}
  };

  GLushort indices[] = {
    0, 1, 2,
    0, 2, 3
  };

  cell_instancing->cell_n_vertices = array_count(vertices);
  cell_instancing->cell_n_indices = array_count(indices);

  cell_instancing->cell_general_vertices_position = opengl_buffer_add_elements(general_vertex_buffer, cell_instancing->cell_n_vertices, vertices);
  cell_instancing->cell_general_indices_position = opengl_buffer_add_elements(general_index_buffer, cell_instancing->cell_n_indices, indices);

  create_opengl_buffer(&cell_instancing->buffer, sizeof(CellInstance), GL_ARRAY_BUFFER, GL_STREAM_DRAW);
}


/// Setup the attributes used by the instanced cell drawing for the cell-instancing.glvs shader.
void
init_cell_instances_buffer_attributes(OpenGL_Buffer *cell_instances_buffer, OpenGL_Buffer *general_vertex_buffer, GLuint cell_instance_drawing_shader_program)
{
  glBindBuffer(general_vertex_buffer->binding_target, general_vertex_buffer->id);

  GLuint attribute_vertex = glGetAttribLocation(cell_instance_drawing_shader_program, "vertex");
  if (attribute_vertex == -1)
  {
    print("Failed to get attribute_vertex\n");
  }
  glEnableVertexAttribArray(attribute_vertex);
  glVertexAttribPointer(attribute_vertex, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 0);


  glBindBuffer(cell_instances_buffer->binding_target, cell_instances_buffer->id);

  GLuint attribute_block_position_x = glGetAttribLocation(cell_instance_drawing_shader_program, "s32_block_position_x");
  if (attribute_block_position_x == -1)
  {
    print("Failed to get attribute_block_position_x\n");
  }
  glEnableVertexAttribArray(attribute_block_position_x);
  glVertexAttribIPointer(attribute_block_position_x, 1, GL_INT, sizeof(CellInstance), (void *)(offsetof(CellInstance, block_position) + offsetof(s32vec2, x)));
  glVertexAttribDivisor(attribute_block_position_x, 1);

  GLuint attribute_block_position_y = glGetAttribLocation(cell_instance_drawing_shader_program, "s32_block_position_y");
  if (attribute_block_position_y == -1)
  {
    print("Failed to get attribute_block_position_y\n");
  }
  glEnableVertexAttribArray(attribute_block_position_y);
  glVertexAttribIPointer(attribute_block_position_y, 1, GL_INT, sizeof(CellInstance), (void *)(offsetof(CellInstance, block_position) + offsetof(s32vec2, y)));
  glVertexAttribDivisor(attribute_block_position_y, 1);

  GLuint attribute_cell_position = glGetAttribLocation(cell_instance_drawing_shader_program, "cell_position");
  if (attribute_cell_position == -1)
  {
    print("Failed to get attribute_cell_position\n");
  }
  glEnableVertexAttribArray(attribute_cell_position);
  glVertexAttribPointer(attribute_cell_position, 2, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void *)offsetof(CellInstance, cell_position));
  glVertexAttribDivisor(attribute_cell_position, 1);

  GLuint attribute_colour = glGetAttribLocation(cell_instance_drawing_shader_program, "cell_colour");
  if (attribute_colour == -1)
  {
    print("Failed to get attribute_colour\n");
  }
  glEnableVertexAttribArray(attribute_colour);
  glVertexAttribPointer(attribute_colour, 4, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void *)offsetof(CellInstance, colour));
  glVertexAttribDivisor(attribute_colour, 1);

  opengl_print_errors();
}


/// @brief Upload all Cells in the Universe to the CellInstancing.buffer so that they can be drawn.
///
/// Overwrites the buffer each call, so any updates are drawn. CellInastances could be updated
///   individually more cleverly, but it isn't worth the complexity while re-uploading them each
///   frame is fast enough.
///
void
upload_cell_instances(Universe *universe, CellInstancing *cell_instancing)
{
  // Zero the buffer
  cell_instancing->buffer.elements_used = 0;

  u32 i = 0;
  for (u32 hash_slot = 0;
       hash_slot < universe->hashmap_size;
       ++hash_slot)
  {
    CellBlock *cell_block = universe->hashmap[hash_slot];

    if (cell_block != 0 && cell_block->initialised)
    {
      do
      {
        for (u32 cell_y = 0;
             cell_y < CELL_BLOCK_DIM;
             ++cell_y)
        {
          for (u32 cell_x = 0;
               cell_x < CELL_BLOCK_DIM;
               ++cell_x)
          {
            Cell *cell = cell_block->cells + (cell_y * CELL_BLOCK_DIM) + cell_x;

            vec2 cell_position = vec2_divide(uvec2_to_vec2(cell->block_offset), CELL_BLOCK_DIM);

            vec4 colour;
            if (cell->state == 1)
            {
              colour = (vec4){0.3, 0.3, 0.3, 1};
            }
            else
            {
              colour = (vec4){0.7, 0.7, 0.7, 1};
            }

            CellInstance cell_instance = {
              .block_position = cell_block->block_position,
              .cell_position = cell_position,
              .colour = colour
            };

            opengl_buffer_new_element(&cell_instancing->buffer, &cell_instance);
          }
        }

        // Follow any hashmap collision chains
        cell_block = cell_block->next_block;
      }
      while (cell_block != 0);
    }
  }
}


/// Draws all the CellInstances uploaded to CellInstance.buffer
void
draw_cell_instances(CellInstancing *cell_instancing)
{
  glBindBuffer(cell_instancing->buffer.binding_target, cell_instancing->buffer.id);
  glDrawElementsInstanced(GL_TRIANGLES, cell_instancing->cell_n_indices, GL_UNSIGNED_SHORT, 0, cell_instancing->buffer.elements_used);

  opengl_print_errors();
}


void
debug_cell_block_outline_drawing_upload(Universe *universe, OpenGL_Buffer *debug_cell_block_outline_drawing_vbo, OpenGL_Buffer *debug_cell_block_outline_drawing_ibo)
{
  u32 i = 0;
  for (u32 hash_slot = 0;
       hash_slot < universe->hashmap_size;
       ++hash_slot)
  {
    CellBlock *cell_block = universe->hashmap[hash_slot];

    if (cell_block != 0 && cell_block->initialised)
    {
      do
      {
        s32vec2 square_vertices[] = {
          vec2_add((s32vec2){0, 0}, cell_block->block_position),
          vec2_add((s32vec2){1, 0}, cell_block->block_position),
          vec2_add((s32vec2){1, 1}, cell_block->block_position),
          vec2_add((s32vec2){0, 1}, cell_block->block_position)
        };

        GLushort index_a = opengl_buffer_new_element(debug_cell_block_outline_drawing_vbo, square_vertices + 0);
        GLushort index_b = opengl_buffer_new_element(debug_cell_block_outline_drawing_vbo, square_vertices + 1);
        GLushort index_c = opengl_buffer_new_element(debug_cell_block_outline_drawing_vbo, square_vertices + 2);
        GLushort index_d = opengl_buffer_new_element(debug_cell_block_outline_drawing_vbo, square_vertices + 3);

        opengl_buffer_new_element(debug_cell_block_outline_drawing_ibo, &index_a);
        opengl_buffer_new_element(debug_cell_block_outline_drawing_ibo, &index_b);
        opengl_buffer_new_element(debug_cell_block_outline_drawing_ibo, &index_b);
        opengl_buffer_new_element(debug_cell_block_outline_drawing_ibo, &index_c);
        opengl_buffer_new_element(debug_cell_block_outline_drawing_ibo, &index_c);
        opengl_buffer_new_element(debug_cell_block_outline_drawing_ibo, &index_d);
        opengl_buffer_new_element(debug_cell_block_outline_drawing_ibo, &index_d);
        opengl_buffer_new_element(debug_cell_block_outline_drawing_ibo, &index_a);

        // Follow any hashmap collision chains
        cell_block = cell_block->next_block;
      }
      while (cell_block != 0);
    }
  }
}


void
debug_cell_block_outline_draw(GLuint vao, OpenGL_Buffer *debug_cell_block_outline_drawing_vbo, OpenGL_Buffer *debug_cell_block_outline_drawing_ibo)
{
  glBindVertexArray(vao);

  glBindBuffer(debug_cell_block_outline_drawing_ibo->binding_target, debug_cell_block_outline_drawing_ibo->id);
  glDrawElements(GL_LINES, debug_cell_block_outline_drawing_ibo->elements_used, GL_UNSIGNED_SHORT, 0);

  opengl_print_errors();
  glBindVertexArray(0);
}
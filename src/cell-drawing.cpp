#include "cell-drawing.h"

#include "util.h"
#include "print.h"
#include "opengl-util.h"
#include "opengl-buffer.h"
#include "opengl-general-buffers.h"

#include "universe.h"
#include "cell-block-coordinate-system.h"
#include "cells-editor.h"

#include <GL/glew.h>

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

  cell_instancing->cell_n_vertices = array_count(vertices);
  cell_instancing->cell_general_vertices_position = opengl_buffer_add_elements(general_vertex_buffer, cell_instancing->cell_n_vertices, vertices);

  GLushort vertices_pos = cell_instancing->cell_general_vertices_position;

  GLushort indices[] = {
    0, 1, 2,
    0, 2, 3
  };

  cell_instancing->cell_n_indices = array_count(indices);

  // These are indices into the general vertex buffer, so they need to be offset to the position of
  //   the vertices in the buffer
  for (u32 i = 0;
       i < cell_instancing->cell_n_indices;
       ++i)
  {
    indices[i] += vertices_pos;
  }

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
upload_cell_instances(Universe *universe, CellInstancing *cell_instancing, CellsEditor *cells_editor)
{
  // Zero the buffer
  cell_instancing->buffer.elements_used = 0;

  for (u32 hash_slot = 0;
       hash_slot < universe->hashmap_size;
       ++hash_slot)
  {
    CellBlock *cell_block = universe->hashmap[hash_slot];

    if (cell_block != 0 && cell_block->slot_in_use)
    {
      do
      {
        s32vec2 cell_position;
        for (cell_position.y = 0;
             cell_position.y < universe->cell_block_dim;
             ++cell_position.y)
        {
          for (cell_position.x = 0;
               cell_position.x < universe->cell_block_dim;
               ++cell_position.x)
          {
            // TODO: Check if block is visible on screen?

            Cell *cell = cell_block->cells + (cell_position.y * universe->cell_block_dim) + cell_position.x;

            UniversePosition current_cell = {
              .cell_block_position = cell_block->block_position,
              .cell_position = vec2_divide((vec2){(r32)cell_position.x, (r32)cell_position.y}, universe->cell_block_dim)
            };

            vec4 colours[] = {(vec4){0x60/255.0, 0x60/255.0, 0x60/255.0},
                              (vec4){0xff/255.0, 0xA0/255.0, 0xA0/255.0},
                              (vec4){0xff/255.0, 0x7d/255.0, 0x00/255.0},
                              (vec4){0xff/255.0, 0x96/255.0, 0x19/255.0},
                              (vec4){0xff/255.0, 0xaf/255.0, 0x32/255.0},
                              (vec4){0xff/255.0, 0xc8/255.0, 0x4b/255.0},
                              (vec4){0xff/255.0, 0xe1/255.0, 0x64/255.0},
                              (vec4){0xff/255.0, 0xfa/255.0, 0x7d/255.0},
                              (vec4){0xfb/255.0, 0xff/255.0, 0x00/255.0},
                              (vec4){0x59/255.0, 0x59/255.0, 0xff/255.0},
                              (vec4){0x6a/255.0, 0x6a/255.0, 0xff/255.0},
                              (vec4){0x7a/255.0, 0x7a/255.0, 0xff/255.0},
                              (vec4){0x8b/255.0, 0x8b/255.0, 0xff/255.0},
                              (vec4){0x1b/255.0, 0xb0/255.0, 0x1b/255.0},
                              (vec4){0x24/255.0, 0xc8/255.0, 0x24/255.0},
                              (vec4){0x49/255.0, 0xff/255.0, 0x49/255.0},
                              (vec4){0x6a/255.0, 0xff/255.0, 0x6a/255.0},
                              (vec4){0xeb/255.0, 0x24/255.0, 0x24/255.0},
                              (vec4){0xff/255.0, 0x38/255.0, 0x38/255.0},
                              (vec4){0xff/255.0, 0x49/255.0, 0x49/255.0},
                              (vec4){0xff/255.0, 0x59/255.0, 0x59/255.0},
                              (vec4){0xb9/255.0, 0x38/255.0, 0xff/255.0},
                              (vec4){0xbf/255.0, 0x49/255.0, 0xff/255.0},
                              (vec4){0xc5/255.0, 0x59/255.0, 0xff/255.0},
                              (vec4){0xcb/255.0, 0x6a/255.0, 0xff/255.0},
                              (vec4){0x00/255.0, 0xff/255.0, 0x80/255.0},
                              (vec4){0xff/255.0, 0x80/255.0, 0x40/255.0},
                              (vec4){0xff/255.0, 0xff/255.0, 0x80/255.0},
                              (vec4){0x21/255.0, 0xd7/255.0, 0xd7/255.0},
                              (vec4){0x1b/255.0, 0xb0/255.0, 0xb0/255.0},
                              (vec4){0x18/255.0, 0x9c/255.0, 0x9c/255.0},
                              (vec4){0x15/255.0, 0x89/255.0, 0x89/255.0}};

            vec4 colour = colours[cell->state % array_count(colours)];

            if (cells_editor->cell_highlighted &&
                cell_position_equal_to(cells_editor->highlighted_cell, current_cell))
            {
              colour = vec4_add(colour, vec4{0.2, 0.2, 0.2, 0});
            }

            CellInstance cell_instance = {
              .block_position = current_cell.cell_block_position,
              .cell_position = current_cell.cell_position,
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

  void *indices_buffer_offset = (void *)(intptr_t)(cell_instancing->cell_general_indices_position * sizeof(GLushort));
  glDrawElementsInstanced(GL_TRIANGLES, cell_instancing->cell_n_indices, GL_UNSIGNED_SHORT, indices_buffer_offset, cell_instancing->buffer.elements_used);

  opengl_print_errors();
}


void
init_debug_cell_block_outline_drawing_attributes(OpenGL_Buffer *debug_cell_block_outline_drawing_vbo, GLuint debug_cell_block_outline_drawing_shader_program)
{
  glBindBuffer(debug_cell_block_outline_drawing_vbo->binding_target, debug_cell_block_outline_drawing_vbo->id);

  GLuint attrib_location_cell_block_position = glGetAttribLocation(debug_cell_block_outline_drawing_shader_program, "s32_cell_block_position");
  glEnableVertexAttribArray(attrib_location_cell_block_position);
  glVertexAttribIPointer(attrib_location_cell_block_position, 2, GL_INT, sizeof(UniversePosition), (void *)(offsetof(UniversePosition, cell_block_position)));

  GLuint attrib_location_cell_position = glGetAttribLocation(debug_cell_block_outline_drawing_shader_program, "r32_cell_position");
  glEnableVertexAttribArray(attrib_location_cell_position);
  glVertexAttribPointer(attrib_location_cell_position, 2, GL_FLOAT, GL_FALSE, sizeof(UniversePosition), (void *)(offsetof(UniversePosition, cell_position)));

  opengl_print_errors();
}


void
debug_cell_block_outline_drawing_upload(Universe *universe, OpenGL_Buffer *debug_cell_block_outline_drawing_vbo, OpenGL_Buffer *debug_cell_block_outline_drawing_ibo)
{
  debug_cell_block_outline_drawing_vbo->elements_used = 0;
  debug_cell_block_outline_drawing_ibo->elements_used = 0;

  for (u32 hash_slot = 0;
       hash_slot < universe->hashmap_size;
       ++hash_slot)
  {
    CellBlock *cell_block = universe->hashmap[hash_slot];

    if (cell_block != 0 && cell_block->slot_in_use)
    {
      do
      {
        UniversePosition square_vertices[] = {
          {cell_block->block_position, {0.05, 0.05}},
          {cell_block->block_position, {0.95, 0.05}},
          {cell_block->block_position, {0.95, 0.95}},
          {cell_block->block_position, {0.05, 0.95}}
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
  opengl_print_errors();
}


void
debug_cell_block_outline_draw(OpenGL_Buffer *debug_cell_block_outline_drawing_vbo, OpenGL_Buffer *debug_cell_block_outline_drawing_ibo)
{
  glBindBuffer(debug_cell_block_outline_drawing_vbo->binding_target, debug_cell_block_outline_drawing_vbo->id);
  glBindBuffer(debug_cell_block_outline_drawing_ibo->binding_target, debug_cell_block_outline_drawing_ibo->id);
  glDrawElements(GL_LINES, debug_cell_block_outline_drawing_ibo->elements_used, GL_UNSIGNED_SHORT, 0);

  opengl_print_errors();
}
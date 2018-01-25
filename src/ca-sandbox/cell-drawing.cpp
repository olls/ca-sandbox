#include "ca-sandbox/cell-drawing.h"

#include "engine/util.h"
#include "engine/print.h"
#include "engine/colour.h"
#include "engine/vectors.h"
#include "engine/opengl-util.h"
#include "engine/opengl-buffer.h"
#include "engine/opengl-general-buffers.h"

#include "ca-sandbox/cell-blocks.h"
#include "ca-sandbox/cell-block-coordinate-system.h"
#include "ca-sandbox/cells-editor.h"
#include "ca-sandbox/named-states.h"

#include <GL/glew.h>

const r32 CELLS_WIDTH = 1;

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
upload_cell_instances(Universe *universe, CellInstancing *cell_instancing, u32 n_highlighted_cells, UniversePosition highlighted_cells[])
{
  for (u32 hash_slot = 0;
       hash_slot < universe->hashmap_size;
       ++hash_slot)
  {
    CellBlock *cell_block = universe->hashmap[hash_slot];

    while (cell_block != 0)
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

          UniversePosition current_cell = {
            .cell_block_position = cell_block->block_position,
            .cell_position = vec2_divide((vec2){(r32)cell_position.x, (r32)cell_position.y}, universe->cell_block_dim)
          };

          CellState cell_state = cell_block->cell_states[(cell_position.y * universe->cell_block_dim) + cell_position.x];

          vec4 colour = get_state_colour(cell_state);

          for (u32 highlighted_cell_index = 0;
               highlighted_cell_index < n_highlighted_cells;
               ++highlighted_cell_index)
          {
            UniversePosition highlighted_cell = highlighted_cells[highlighted_cell_index];
            if (cell_position_equal_to(highlighted_cell, current_cell))
            {
              colour = lighten_colour(colour);
              break;
            }
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
draw_cell_blocks(CellBlocks *cell_blocks,
                 CellInstancing *cell_instancing,
                 GLuint cell_instance_drawing_vao,
                 GLuint cell_instance_drawing_shader_program,
                 GLuint cell_instance_drawing_mat4_projection_matrix_uniform,
                 GLuint cell_instance_drawing_cell_block_dim_uniform,
                 GLuint cell_instance_drawing_cell_width_uniform,
                 OpenGL_Buffer *general_vertex_buffer,
                 mat4x4 projection_matrix,
                 u32 n_highlighted_cells, UniversePosition highlighted_cells[])
{
  cell_instancing->buffer.elements_used = 0;
  upload_cell_instances(cell_blocks, cell_instancing, n_highlighted_cells, highlighted_cells);

  glBindVertexArray(cell_instance_drawing_vao);
  glUseProgram(cell_instance_drawing_shader_program);

  mat4x4 projection_matrix_t;
  mat4x4Transpose(projection_matrix_t, projection_matrix);
  glUniformMatrix4fv(cell_instance_drawing_mat4_projection_matrix_uniform, 1, GL_TRUE, &projection_matrix_t[0][0]);

  glUniform1i(cell_instance_drawing_cell_block_dim_uniform, cell_blocks->cell_block_dim);
  glUniform1f(cell_instance_drawing_cell_width_uniform, CELLS_WIDTH);

  // Re-initialise attributes in case instance buffer has been reallocated
  init_cell_instances_buffer_attributes(&cell_instancing->buffer, general_vertex_buffer, cell_instance_drawing_shader_program);

  draw_cell_instances(cell_instancing);

  opengl_print_errors();
  glBindVertexArray(0);
}


void
init_general_universe_attributes(OpenGL_Buffer *general_universe_vbo, GLuint general_universe_shader_program)
{
  glBindBuffer(general_universe_vbo->binding_target, general_universe_vbo->id);

  GLuint attrib_location_cell_block_position = glGetAttribLocation(general_universe_shader_program, "s32_cell_block_position");
  glEnableVertexAttribArray(attrib_location_cell_block_position);
  glVertexAttribIPointer(attrib_location_cell_block_position, 2, GL_INT, sizeof(GeneralUnvierseVertex),
                         (void *)(offsetof(GeneralUnvierseVertex, vertex) + offsetof(UniversePosition, cell_block_position)));

  GLuint attrib_location_cell_position = glGetAttribLocation(general_universe_shader_program, "r32_cell_position");
  glEnableVertexAttribArray(attrib_location_cell_position);
  glVertexAttribPointer(attrib_location_cell_position, 2, GL_FLOAT, GL_FALSE, sizeof(GeneralUnvierseVertex),
                        (void *)(offsetof(GeneralUnvierseVertex, vertex) + offsetof(UniversePosition, cell_position)));

  GLuint attrib_location_colour = glGetAttribLocation(general_universe_shader_program, "colour");
  glEnableVertexAttribArray(attrib_location_colour);
  glVertexAttribPointer(attrib_location_colour, 4, GL_FLOAT, GL_FALSE, sizeof(GeneralUnvierseVertex), (void *)(offsetof(GeneralUnvierseVertex, colour)));

  opengl_print_errors();
}


u32
debug_cell_block_outline_drawing_upload(Universe *universe, OpenGL_Buffer *general_universe_vbo, OpenGL_Buffer *general_universe_ibo)
{
  u32 index_elements_used = 0;

  for (u32 hash_slot = 0;
       hash_slot < universe->hashmap_size;
       ++hash_slot)
  {
    CellBlock *cell_block = universe->hashmap[hash_slot];

    while (cell_block != 0)
    {
      vec4 red = {1, 0, 0, 1};
      GeneralUnvierseVertex square_vertices[] = {
        {{cell_block->block_position, {0.05, 0.05}}, red},
        {{cell_block->block_position, {0.95, 0.05}}, red},
        {{cell_block->block_position, {0.95, 0.95}}, red},
        {{cell_block->block_position, {0.05, 0.95}}, red}
      };

      GLushort vbo_index_a = opengl_buffer_new_element(general_universe_vbo, square_vertices + 0);
      GLushort vbo_index_b = opengl_buffer_new_element(general_universe_vbo, square_vertices + 1);
      GLushort vbo_index_c = opengl_buffer_new_element(general_universe_vbo, square_vertices + 2);
      GLushort vbo_index_d = opengl_buffer_new_element(general_universe_vbo, square_vertices + 3);

      opengl_buffer_new_element(general_universe_ibo, &vbo_index_a);
      opengl_buffer_new_element(general_universe_ibo, &vbo_index_b);
      opengl_buffer_new_element(general_universe_ibo, &vbo_index_b);
      opengl_buffer_new_element(general_universe_ibo, &vbo_index_c);
      opengl_buffer_new_element(general_universe_ibo, &vbo_index_c);
      opengl_buffer_new_element(general_universe_ibo, &vbo_index_d);
      opengl_buffer_new_element(general_universe_ibo, &vbo_index_d);
      opengl_buffer_new_element(general_universe_ibo, &vbo_index_a);

      index_elements_used += 8;

      // Follow any hashmap collision chains
      cell_block = cell_block->next_block;
    }
  }
  opengl_print_errors();

  return index_elements_used;
}


void
debug_lines_draw(OpenGL_Buffer *general_universe_vbo, OpenGL_Buffer *general_universe_ibo, BufferDrawingLocation ibo_outline_elements)
{
  glBindBuffer(general_universe_vbo->binding_target, general_universe_vbo->id);
  glBindBuffer(general_universe_ibo->binding_target, general_universe_ibo->id);

  void *general_universe_ibo_offset = (void *)(intptr_t)(ibo_outline_elements.start_position * sizeof(GLushort));
  glDrawElements(GL_LINES, ibo_outline_elements.n_elements, GL_UNSIGNED_SHORT, general_universe_ibo_offset);

  opengl_print_errors();
}
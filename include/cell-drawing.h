#ifndef CELL_DRAWING_H_DEF
#define CELL_DRAWING_H_DEF

#include "opengl-buffer.h"
#include "cell-storage.h"


struct CellInstancing
{
  OpenGL_Buffer buffer;

  u32 cell_general_vertices_position;
  u32 cell_general_indices_position;

  u32 cell_n_vertices;
  u32 cell_n_indices;
};


struct CellInstance
{
  s32vec2 block_position;

  // The cell's position within the block 0-1
  vec2 cell_position;

  vec4 colour;
};


void
init_cell_drawing(CellInstancing *cell_instancing, OpenGL_Buffer *general_vertex_buffer, OpenGL_Buffer *general_index_buffer);


void
init_cell_instances_buffer_attributes(OpenGL_Buffer *cell_instances_buffer, OpenGL_Buffer *general_vertex_buffer, OpenGL_Buffer *general_index_buffer, GLuint cell_instance_drawing_shader_program);


void
upload_cell_instances(Universe *universe, CellInstancing *cell_instancing);


void
draw_cell_instances(CellInstancing *cell_instancing);


void
test_draw_cell_blocks_upload(Universe *universe, OpenGL_Buffer *cell_drawing_vbo, OpenGL_Buffer *cell_drawing_ibo);


void
test_draw_cell_blocks(GLuint vao, OpenGL_Buffer *cell_drawing_vbo, OpenGL_Buffer *cell_drawing_ibo);


#endif
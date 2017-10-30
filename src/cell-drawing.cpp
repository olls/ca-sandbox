#include <GL/glew.h>

#include "print.h"
#include "opengl-util.h"
#include "opengl-buffer.h"
#include "cell-drawing.h"
#include "cell-storage.h"


void
test_draw_cells_upload(Universe *universe, OpenGL_Buffer *cell_drawing_vbo, OpenGL_Buffer *cell_drawing_ibo)
{
  u32 i = 0;
  for (u32 hash_slot = 0; hash_slot < universe->hashmap_size; ++hash_slot)
  {
    CellBlock *cell_block = universe->hashmap[hash_slot];

    if (cell_block != 0 && cell_block->initialised)
    {
      s32vec2 square_vertices[] = {
        vec2_add((s32vec2){0, 0}, cell_block->block_position),
        vec2_add((s32vec2){1, 0}, cell_block->block_position),
        vec2_add((s32vec2){1, 1}, cell_block->block_position),
        vec2_add((s32vec2){0, 1}, cell_block->block_position)
      };

      GLushort index_a = opengl_buffer_new_element(cell_drawing_vbo, square_vertices + 0);
      GLushort index_b = opengl_buffer_new_element(cell_drawing_vbo, square_vertices + 1);
      GLushort index_c = opengl_buffer_new_element(cell_drawing_vbo, square_vertices + 2);
      GLushort index_d = opengl_buffer_new_element(cell_drawing_vbo, square_vertices + 3);

      opengl_buffer_new_element(cell_drawing_ibo, &index_a);
      opengl_buffer_new_element(cell_drawing_ibo, &index_b);
      opengl_buffer_new_element(cell_drawing_ibo, &index_c);

      opengl_buffer_new_element(cell_drawing_ibo, &index_d);
      opengl_buffer_new_element(cell_drawing_ibo, &index_a);
      opengl_buffer_new_element(cell_drawing_ibo, &index_c);
    }
  }
}


void
test_draw_cell_blocks(GLuint vao, OpenGL_Buffer *cell_drawing_vbo, OpenGL_Buffer *cell_drawing_ibo)
{
  glBindVertexArray(vao);

  glBindBuffer(cell_drawing_ibo->binding_target, cell_drawing_ibo->id);
  glDrawElements(GL_TRIANGLES, cell_drawing_ibo->elements_used, GL_UNSIGNED_SHORT, 0);

  opengl_print_errors();
  glBindVertexArray(0);
}
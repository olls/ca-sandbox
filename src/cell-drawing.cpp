#include <GL/glew.h>

#include "print.h"
#include "opengl-util.h"
#include "opengl-buffer.h"
#include "cell-drawing.h"
#include "cell-storage.h"


void
test_draw_cells_upload(Universe *universe, OpenGL_Buffer *cell_drawing_vbo)
{
  for (u32 hash_slot = 0; hash_slot < universe->hashmap_size; ++hash_slot)
  {
    CellBlock *cell_block = universe->hashmap[hash_slot];

    if (cell_block != 0 && cell_block->initialised)
    {
      s32Vec2 square_vertices[] = {
        (s32Vec2){0, 0} + cell_block->block_position,
        (s32Vec2){1, 0} + cell_block->block_position,
        (s32Vec2){1, 1} + cell_block->block_position,
        (s32Vec2){0, 1} + cell_block->block_position
      };

      opengl_buffer_new_element(cell_drawing_vbo, square_vertices + 0);
      opengl_buffer_new_element(cell_drawing_vbo, square_vertices + 1);
      opengl_buffer_new_element(cell_drawing_vbo, square_vertices + 2);
      opengl_buffer_new_element(cell_drawing_vbo, square_vertices + 3);
    }
  }
}


void
test_draw_cells(GLuint shader_program, GLuint vao, OpenGL_Buffer *cell_drawing_vbo)
{
  glUseProgram(shader_program);
  glBindVertexArray(vao);
  glBindBuffer(cell_drawing_vbo->binding_target, cell_drawing_vbo->id);

  glDrawArrays(GL_LINE_LOOP, 0, cell_drawing_vbo->elements_used);

  opengl_print_errors();

  glBindVertexArray(0);
  glBindBuffer(cell_drawing_vbo->binding_target, 0);
}
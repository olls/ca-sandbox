#ifndef CELL_DRAWING_H_DEF
#define CELL_DRAWING_H_DEF

#include "opengl-buffer.h"
#include "cell-storage.h"


void
test_draw_cell_blocks_upload(Universe *universe, OpenGL_Buffer *cell_drawing_vbo, OpenGL_Buffer *cell_drawing_ibo);


void
test_draw_cell_blocks(GLuint vao, OpenGL_Buffer *cell_drawing_vbo, OpenGL_Buffer *cell_drawing_ibo);


#endif
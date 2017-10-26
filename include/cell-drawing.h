#ifndef CELL_DRAWING_H_DEF
#define CELL_DRAWING_H_DEF

#include "opengl-buffer.h"
#include "cell-storage.h"


void
test_draw_cells_upload(Universe *universe, OpenGL_Buffer *cell_drawing_vbo);


void
test_draw_cells(GLuint shader_program, GLuint vao, OpenGL_Buffer *cell_drawing_vbo);


#endif
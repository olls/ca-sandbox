#ifndef MINIMAP_H_DEF
#define MINIMAP_H_DEF

#include "ca-sandbox/cell-blocks.h"
#include "ca-sandbox/cell-drawing.h"

#include "engine/vectors.h"
#include "engine/opengl-buffer.h"

#include <GL/glew.h>


GLuint
create_minimap_framebuffer();


GLuint
create_minimap_texture(s32vec2 size, GLuint framebuffer);


void
draw_minimap_texture(CellBlocks *cell_blocks, CellInstancing *cell_instancing, CellDrawing *cell_drawing, OpenGL_Buffer *general_vertex_buffer, GLuint framebuffer, GLuint texture, s32vec2 texture_size);


void
draw_minimap_texture_to_screen(GLuint framebuffer, GLuint texture, s32vec2 texture_size, GLuint texture_shader, GLuint rendered_texture_uniform);


#endif
#ifndef CELL_DRAWING_H_DEF
#define CELL_DRAWING_H_DEF

#include "ca-sandbox/cell-blocks.h"
#include "ca-sandbox/cells-editor.h"
#include "ca-sandbox/border.h"

#include "engine/vectors.h"
#include "engine/opengl-buffer.h"
#include "engine/drawing.h"

/// @file
/// @brief Data structures for drawing the Cell%s with OpenGL
///
/// Cells are drawn using OpenGL instanced rendering. The vertices are uploaded once to an
///   OpenGL_Buffer, then the coordinates and colour for each CellInstance is uploaded separately.
///   All the CellInstance%s are then drawn by OpenGL using the one set of vertices.
///
/// The cell vertices and indices are stored in the `general_buffer`, which is maintained in the
///   main() function.
///


/// Holds the OpenGL identifiers for cell drawing
struct CellInstancing
{
  /// The buffer where the CellInstances are uploaded
  OpenGL_Buffer buffer;

  /// The position of the cell vertices within the general buffer
  u32 cell_general_vertices_position;
  u32 cell_n_vertices;  ///< Number of vertices stored in the general buffer

  /// The position of the cell vertex indices within the general buffer
  u32 cell_general_indices_position;
  u32 cell_n_indices;  ///< Number of indices stored in the general buffer
};


/// @brief The details for a single cell to be rendered, this is the object which is uploaded to the
///          OpenGL_Buffer
///
struct CellInstance
{
  /// The global position of the block
  s32vec2 block_position;

  /// The cell's position within the block 0-1
  vec2 cell_position;

  /// Colour to draw the cell
  vec4 colour;
};


struct CellDrawing
{
  GLuint vao;
  GLuint shader_program;
  GLuint mat4_projection_matrix_uniform;
  GLuint cell_block_dim_uniform;
  GLuint cell_width_uniform;
};


struct GeneralUnvierseVertex
{
  UniversePosition vertex;
  vec4 colour;
};


b32
init_cell_drawing_shaders(CellDrawing *cell_drawing);


void
init_cell_drawing(CellDrawing *cell_drawing, CellInstancing *cell_instancing, OpenGL_Buffer *general_vertex_buffer, OpenGL_Buffer *general_index_buffer);


void
init_cell_instances_buffer_attributes(OpenGL_Buffer *cell_instances_buffer, OpenGL_Buffer *general_vertex_buffer, GLuint cell_instance_drawing_shader_program);


void
upload_cell_instances(Universe *universe, Border border, CellInstancing *cell_instancing);


void
draw_cell_instances(CellInstancing *cell_instancing);


void
draw_cell_blocks(CellBlocks *cell_blocks,
                 CellInstancing *cell_instancing,
                 CellDrawing *cell_drawing,
                 OpenGL_Buffer *general_vertex_buffer,
                 mat4x4 projection_matrix);


void
init_general_universe_attributes(OpenGL_Buffer *general_universe_vbo, GLuint general_universe_shader_program);


u32
debug_cell_block_outline_drawing_upload(Universe *universe, OpenGL_Buffer *cell_drawing_vbo, OpenGL_Buffer *cell_drawing_ibo);


void
debug_lines_draw(OpenGL_Buffer *general_universe_vbo, OpenGL_Buffer *general_universe_ibo, BufferDrawingLocation ibo_elements);


#endif
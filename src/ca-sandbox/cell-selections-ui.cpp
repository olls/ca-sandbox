#include "ca-sandbox/cell-selections-ui.h"

#include "imgui/imgui.h"


void
do_cell_selections_ui(CellSelectionsUI *cell_selections_ui, UniversePosition mouse_universe_pos, b32 *mouse_click_consumed)
{
  if (!*mouse_click_consumed)
  {
    // Remove selection on mouse click anywhere
    if (ImGui::IsMouseClicked(0) && cell_selections_ui->selection_made)
    {
      *mouse_click_consumed = true;
      cell_selections_ui->selection_made = false;
    }

    ImGuiIO& io = ImGui::GetIO();
    if (io.KeyShift)
    {
      if (ImGui::IsMouseClicked(0))
      {
        cell_selections_ui->making_selection = true;
        cell_selections_ui->selection_made = false;

        cell_selections_ui->selection_start = mouse_universe_pos;
        cell_selections_ui->selection_end = mouse_universe_pos;
      }

      if (cell_selections_ui->making_selection &&
          ImGui::IsMouseDragging(0))
      {
        cell_selections_ui->selection_end = mouse_universe_pos;
      }

      if (cell_selections_ui->making_selection)
      {
        *mouse_click_consumed = true;
      }
    }
    if (!ImGui::IsMouseDown(0) && cell_selections_ui->making_selection)
    {
      cell_selections_ui->making_selection = false;
      cell_selections_ui->selection_made = true;

      correct_cell_block_square_order(cell_selections_ui->selection_start, cell_selections_ui->selection_end);
    }
  }
}


void
correct_square_order(r32& start, r32& end)
{
  if (start >= end)
  {
    r32 temp_end = end;
    end = start;
    start = temp_end;
  }
}

void
correct_square_order(vec2& start, vec2& end)
{
  correct_square_order(start.x, end.x);
  correct_square_order(start.y, end.y);
}


/// Draws triangles depicting the currently selected region in the UniverseUI
///
void
cell_selections_drawing_upload(CellSelectionsUI *cell_selections_ui, Universe *universe, mat4x4 universe_projection_matrix, mat4x4 aspect_ratio, OpenGL_Buffer *general_vbo, OpenGL_Buffer *general_ibo, BufferDrawingLocation *selection_vbo_position, BufferDrawingLocation *selection_ibo_position)
{
  UniversePosition inner_start_corner_universe = cell_selections_ui->selection_start;
  UniversePosition inner_end_corner_universe = cell_selections_ui->selection_end;

  quantise_0to1_cell_position(inner_start_corner_universe.cell_position, universe->cell_block_dim);
  quantise_0to1_cell_position(inner_end_corner_universe.cell_position, universe->cell_block_dim);

  // Convert to screen coordinates
  vec4 inner_start_corner_universe_v4 = {0,0,0,1};
  inner_start_corner_universe_v4.xy = vec2_add(s32vec2_to_vec2(inner_start_corner_universe.cell_block_position), inner_start_corner_universe.cell_position);
  vec4 inner_end_corner_universe_v4 = {0,0,0,1};
  inner_end_corner_universe_v4.xy = vec2_add(s32vec2_to_vec2(inner_end_corner_universe.cell_block_position), inner_end_corner_universe.cell_position);

  mat4x4 aspect_ratio_inv;
  mat4x4Inverse(aspect_ratio_inv, aspect_ratio);

  vec4 inner_start_corner_v4 = mat4x4MultiplyVector(universe_projection_matrix, inner_start_corner_universe_v4);
  vec2 inner_start_corner = mat4x4MultiplyVector(aspect_ratio_inv, inner_start_corner_v4).xy;

  vec4 inner_end_corner_v4 = mat4x4MultiplyVector(universe_projection_matrix, inner_end_corner_universe_v4);
  vec2 inner_end_corner = mat4x4MultiplyVector(aspect_ratio_inv, inner_end_corner_v4).xy;

  correct_square_order(inner_start_corner, inner_end_corner);

  vec2 inner_start_end_corner = {inner_start_corner.x, inner_end_corner.y};
  vec2 inner_end_start_corner = {inner_end_corner.x, inner_start_corner.y};

  r32 border_width = 0.015;
  vec2 outer_start_corner = vec2_add(inner_start_corner, -border_width);
  vec2 outer_end_corner = vec2_add(inner_end_corner, border_width);
  vec2 outer_start_end_corner = {outer_start_corner.x, outer_end_corner.y};
  vec2 outer_end_start_corner = {outer_end_corner.x, outer_start_corner.y};

  selection_vbo_position->start_position = general_vbo->elements_used;

  GLushort inner_start_vertex     = opengl_buffer_new_element(general_vbo, &inner_start_corner);
  GLushort inner_start_end_vertex = opengl_buffer_new_element(general_vbo, &inner_start_end_corner);
  GLushort inner_end_vertex       = opengl_buffer_new_element(general_vbo, &inner_end_corner);
  GLushort inner_end_start_vertex = opengl_buffer_new_element(general_vbo, &inner_end_start_corner);

  GLushort outer_start_vertex     = opengl_buffer_new_element(general_vbo, &outer_start_corner);
  GLushort outer_start_end_vertex = opengl_buffer_new_element(general_vbo, &outer_start_end_corner);
  GLushort outer_end_vertex       = opengl_buffer_new_element(general_vbo, &outer_end_corner);
  GLushort outer_end_start_vertex = opengl_buffer_new_element(general_vbo, &outer_end_start_corner);

  selection_vbo_position->n_elements = general_vbo->elements_used - selection_vbo_position->start_position;

  selection_ibo_position->start_position = general_ibo->elements_used;

  // Left
  opengl_buffer_new_element(general_ibo, &inner_start_vertex);
  opengl_buffer_new_element(general_ibo, &inner_start_end_vertex);
  opengl_buffer_new_element(general_ibo, &outer_start_vertex);

  opengl_buffer_new_element(general_ibo, &outer_start_vertex);
  opengl_buffer_new_element(general_ibo, &outer_start_end_vertex);
  opengl_buffer_new_element(general_ibo, &inner_start_end_vertex);

  // Right
  opengl_buffer_new_element(general_ibo, &inner_end_vertex);
  opengl_buffer_new_element(general_ibo, &inner_end_start_vertex);
  opengl_buffer_new_element(general_ibo, &outer_end_vertex);

  opengl_buffer_new_element(general_ibo, &outer_end_vertex);
  opengl_buffer_new_element(general_ibo, &outer_end_start_vertex);
  opengl_buffer_new_element(general_ibo, &inner_end_start_vertex);

  // Top
  opengl_buffer_new_element(general_ibo, &inner_end_vertex);
  opengl_buffer_new_element(general_ibo, &inner_start_end_vertex);
  opengl_buffer_new_element(general_ibo, &outer_end_vertex);

  opengl_buffer_new_element(general_ibo, &outer_end_vertex);
  opengl_buffer_new_element(general_ibo, &outer_start_end_vertex);
  opengl_buffer_new_element(general_ibo, &inner_start_end_vertex);

  // Bottom
  opengl_buffer_new_element(general_ibo, &inner_start_vertex);
  opengl_buffer_new_element(general_ibo, &inner_end_start_vertex);
  opengl_buffer_new_element(general_ibo, &outer_start_vertex);

  opengl_buffer_new_element(general_ibo, &outer_start_vertex);
  opengl_buffer_new_element(general_ibo, &outer_end_start_vertex);
  opengl_buffer_new_element(general_ibo, &inner_end_start_vertex);

  selection_ibo_position->n_elements = general_ibo->elements_used - selection_ibo_position->start_position;
}


/// Generates the two other coordinates of a square given the start and end position (in UniversePosition%s)
///
void
square_universe_coordinates(UniversePosition& start, UniversePosition& end, UniversePosition *start_end, UniversePosition *end_start)
{
  *start_end = {{start.cell_block_position.x, end.cell_block_position.y},
                {start.cell_position.x, end.cell_position.y}};
  *end_start = {{end.cell_block_position.x, start.cell_block_position.y},
                {end.cell_position.x, start.cell_position.y}};
}


u32
debug_cell_selections_drawing_upload(CellSelectionsUI *cell_selections_ui, Universe *universe, OpenGL_Buffer *general_universe_vbo, OpenGL_Buffer *general_universe_ibo)
{
  UniversePosition start_corner = cell_selections_ui->selection_start;
  UniversePosition end_corner = cell_selections_ui->selection_end;

  quantise_0to1_cell_position(start_corner.cell_position, universe->cell_block_dim);
  quantise_0to1_cell_position(end_corner.cell_position, universe->cell_block_dim);

  correct_cell_block_square_order(start_corner, end_corner);

  UniversePosition start_end_corner;
  UniversePosition end_start_corner;
  square_universe_coordinates(start_corner, end_corner, &start_end_corner, &end_start_corner);

  vec4 colour = {0.3, 0.3, 0.3, 1};
  GeneralUnvierseVertex vertex_a = {start_corner, colour};
  GeneralUnvierseVertex vertex_b = {start_end_corner, colour};
  GeneralUnvierseVertex vertex_c = {end_corner, colour};
  GeneralUnvierseVertex vertex_d = {end_start_corner, colour};

  u32 vertex_a_pos = opengl_buffer_new_element(general_universe_vbo, &vertex_a);
  u32 vertex_b_pos = opengl_buffer_new_element(general_universe_vbo, &vertex_b);
  u32 vertex_c_pos = opengl_buffer_new_element(general_universe_vbo, &vertex_c);
  u32 vertex_d_pos = opengl_buffer_new_element(general_universe_vbo, &vertex_d);

  u32 start_pos = opengl_buffer_new_element(general_universe_ibo, &vertex_a_pos);
  opengl_buffer_new_element(general_universe_ibo, &vertex_b_pos);

  opengl_buffer_new_element(general_universe_ibo, &vertex_b_pos);
  opengl_buffer_new_element(general_universe_ibo, &vertex_c_pos);

  opengl_buffer_new_element(general_universe_ibo, &vertex_c_pos);
  opengl_buffer_new_element(general_universe_ibo, &vertex_d_pos);

  opengl_buffer_new_element(general_universe_ibo, &vertex_d_pos);
  u32 end_pos = opengl_buffer_new_element(general_universe_ibo, &vertex_a_pos);

  return (end_pos - start_pos) + 1;
}

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
cell_selections_drawing_upload(CellSelectionsUI *cell_selections_ui, Universe *universe, mat4x4 universe_projection_matrix, mat4x4 aspect_ratio,
                               OpenGL_Buffer *vertex_buffer, OpenGL_Buffer *colour_buffer, OpenGL_Buffer *ibo,
                               BufferDrawingLocation *vertices_position, BufferDrawingLocation *colours_position, BufferDrawingLocation *ibo_position)
{
  UniversePosition start_corner_universe = cell_selections_ui->selection_start;
  UniversePosition end_corner_universe = cell_selections_ui->selection_end;

  quantise_0to1_cell_position(start_corner_universe.cell_position, universe->cell_block_dim);
  quantise_0to1_cell_position(end_corner_universe.cell_position, universe->cell_block_dim);

  // Convert to screen coordinates
  vec4 start_corner_universe_v4 = {0,0,0,1};
  start_corner_universe_v4.xy = vec2_add(s32vec2_to_vec2(start_corner_universe.cell_block_position), start_corner_universe.cell_position);
  vec4 end_corner_universe_v4 = {0,0,0,1};
  end_corner_universe_v4.xy = vec2_add(s32vec2_to_vec2(end_corner_universe.cell_block_position), end_corner_universe.cell_position);

  mat4x4 aspect_ratio_inv;
  mat4x4Inverse(aspect_ratio_inv, aspect_ratio);

  vec4 start_corner_v4 = mat4x4MultiplyVector(universe_projection_matrix, start_corner_universe_v4);
  vec2 start_corner = mat4x4MultiplyVector(aspect_ratio_inv, start_corner_v4).xy;

  vec4 end_corner_v4 = mat4x4MultiplyVector(universe_projection_matrix, end_corner_universe_v4);
  vec2 end_corner = mat4x4MultiplyVector(aspect_ratio_inv, end_corner_v4).xy;

  correct_square_order(start_corner, end_corner);

  vec2 inner_border_size = {0.005, 0.005};
  vec2 outer_border_size = {0.010, 0.010};

  vertices_position->start_position = vertex_buffer->elements_used;
  ibo_position->start_position = ibo->elements_used;

  u32 n_vertices_outer = upload_outline(start_corner, end_corner, outer_border_size, vertex_buffer, ibo,
                                        &vertices_position->n_elements, &ibo_position->n_elements);

  u32 n_vertices_inner = upload_outline(start_corner, end_corner, inner_border_size, vertex_buffer, ibo,
                                        &vertices_position->n_elements, &ibo_position->n_elements);

  vec4 inner_colour = {0.8, 0.8, 0.8, 1};
  vec4 outer_colour = {0.5, 0.5, 0.5, 1};

  for (u32 i = 0;
       i < n_vertices_outer;
       ++i)
  {
    opengl_buffer_new_element(colour_buffer, &outer_colour);
    colours_position->n_elements += 1;
  }

  for (u32 i = 0;
       i < n_vertices_inner;
       ++i)
  {
    opengl_buffer_new_element(colour_buffer, &inner_colour);
    colours_position->n_elements += 1;
  }
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

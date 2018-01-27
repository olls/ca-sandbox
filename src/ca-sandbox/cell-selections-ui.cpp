#include "ca-sandbox/cell-selections-ui.h"

#include "imgui/imgui.h"


void
do_cell_selections_ui(CellSelectionsUI *cell_selections_ui, UniversePosition mouse_universe_pos, b32 *mouse_click_consumed)
{
  if (!*mouse_click_consumed)
  {
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
        *mouse_click_consumed = true;
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

    }
    if (!ImGui::IsMouseDown(0) && cell_selections_ui->making_selection)
    {
      cell_selections_ui->making_selection = false;
      cell_selections_ui->selection_made = true;

      correct_cell_block_square_order(cell_selections_ui->selection_start, cell_selections_ui->selection_end);
    }
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


/// Draws triangles depicting the currently selected region in the UniverseUI
///
u32
cell_selections_drawing_upload(CellSelectionsUI *cell_selections_ui, Universe *universe, OpenGL_Buffer *general_universe_vbo, OpenGL_Buffer *general_universe_ibo)
{
  UniversePosition inner_start_corner = cell_selections_ui->selection_start;
  UniversePosition inner_end_corner = cell_selections_ui->selection_end;

  quantise_0to1_cell_position(inner_start_corner.cell_position, universe->cell_block_dim);
  quantise_0to1_cell_position(inner_end_corner.cell_position, universe->cell_block_dim);

  correct_cell_block_square_order(inner_start_corner, inner_end_corner);

  UniversePosition inner_start_end_corner;
  UniversePosition inner_end_start_corner;
  square_universe_coordinates(inner_start_corner, inner_end_corner, &inner_start_end_corner, &inner_end_start_corner);

  r32 border_width = 0.05;
  UniversePosition outer_start_corner = {inner_start_corner.cell_block_position, vec2_add(inner_start_corner.cell_position, -border_width)};
  UniversePosition outer_end_corner = {inner_end_corner.cell_block_position, vec2_add(inner_end_corner.cell_position, border_width)};

  UniversePosition outer_start_end_corner;
  UniversePosition outer_end_start_corner;
  square_universe_coordinates(outer_start_corner, outer_end_corner, &outer_start_end_corner, &outer_end_start_corner);

  vec4 inner_colour = {0.5, 0.5, 0.5, 1};
  GeneralUnvierseVertex vertex_inner_start     = {inner_start_corner, inner_colour};
  GeneralUnvierseVertex vertex_inner_start_end = {inner_start_end_corner, inner_colour};
  GeneralUnvierseVertex vertex_inner_end       = {inner_end_corner, inner_colour};
  GeneralUnvierseVertex vertex_inner_end_start = {inner_end_start_corner, inner_colour};

  vec4 outer_colour = {0.6, 0.6, 0.6, 1};
  GeneralUnvierseVertex vertex_outer_start     = {outer_start_corner, outer_colour};
  GeneralUnvierseVertex vertex_outer_start_end = {outer_start_end_corner, outer_colour};
  GeneralUnvierseVertex vertex_outer_end       = {outer_end_corner, outer_colour};
  GeneralUnvierseVertex vertex_outer_end_start = {outer_end_start_corner, outer_colour};

  u32 vertex_inner_start_pos     = opengl_buffer_new_element(general_universe_vbo, &vertex_inner_start);
  u32 vertex_inner_start_end_pos = opengl_buffer_new_element(general_universe_vbo, &vertex_inner_start_end);
  u32 vertex_inner_end_pos       = opengl_buffer_new_element(general_universe_vbo, &vertex_inner_end);
  u32 vertex_inner_end_start_pos = opengl_buffer_new_element(general_universe_vbo, &vertex_inner_end_start);

  u32 vertex_outer_start_pos     = opengl_buffer_new_element(general_universe_vbo, &vertex_outer_start);
  u32 vertex_outer_start_end_pos = opengl_buffer_new_element(general_universe_vbo, &vertex_outer_start_end);
  u32 vertex_outer_end_pos       = opengl_buffer_new_element(general_universe_vbo, &vertex_outer_end);
  u32 vertex_outer_end_start_pos = opengl_buffer_new_element(general_universe_vbo, &vertex_outer_end_start);

  u32 start_pos = general_universe_ibo->elements_used;

  // Top edge
  opengl_buffer_new_element(general_universe_ibo, &vertex_inner_start_pos);
  opengl_buffer_new_element(general_universe_ibo, &vertex_inner_end_start_pos);
  opengl_buffer_new_element(general_universe_ibo, &vertex_outer_start_pos);

  opengl_buffer_new_element(general_universe_ibo, &vertex_outer_start_pos);
  opengl_buffer_new_element(general_universe_ibo, &vertex_inner_end_start_pos);
  opengl_buffer_new_element(general_universe_ibo, &vertex_outer_end_start_pos);

  // Right edge
  opengl_buffer_new_element(general_universe_ibo, &vertex_outer_end_pos);
  opengl_buffer_new_element(general_universe_ibo, &vertex_inner_end_pos);
  opengl_buffer_new_element(general_universe_ibo, &vertex_inner_end_start_pos);

  opengl_buffer_new_element(general_universe_ibo, &vertex_outer_end_pos);
  opengl_buffer_new_element(general_universe_ibo, &vertex_inner_end_start_pos);
  opengl_buffer_new_element(general_universe_ibo, &vertex_outer_end_start_pos);

  // Bottom edge
  opengl_buffer_new_element(general_universe_ibo, &vertex_inner_start_end_pos);
  opengl_buffer_new_element(general_universe_ibo, &vertex_inner_end_pos);
  opengl_buffer_new_element(general_universe_ibo, &vertex_outer_start_end_pos);

  opengl_buffer_new_element(general_universe_ibo, &vertex_outer_start_end_pos);
  opengl_buffer_new_element(general_universe_ibo, &vertex_inner_end_pos);
  opengl_buffer_new_element(general_universe_ibo, &vertex_outer_end_pos);

  // Left edge
  opengl_buffer_new_element(general_universe_ibo, &vertex_inner_start_pos);
  opengl_buffer_new_element(general_universe_ibo, &vertex_outer_start_pos);
  opengl_buffer_new_element(general_universe_ibo, &vertex_inner_start_end_pos);

  opengl_buffer_new_element(general_universe_ibo, &vertex_outer_start_pos);
  opengl_buffer_new_element(general_universe_ibo, &vertex_outer_start_end_pos);
  opengl_buffer_new_element(general_universe_ibo, &vertex_inner_start_end_pos);

  u32 end_pos = general_universe_ibo->elements_used;

  return (end_pos - start_pos);
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

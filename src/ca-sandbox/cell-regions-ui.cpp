#include "ca-sandbox/cell-regions-ui.h"

#include "ca-sandbox/cell-block-coordinate-system.h"
#include "ca-sandbox/cell-regions.h"
#include "ca-sandbox/universe.h"
#include "ca-sandbox/cell-drawing.h"

#include "engine/opengl-buffer.h"
#include "engine/vectors.h"

#include "imgui/imgui.h"


void
do_cell_regions_ui(CellRegionsUI *cell_regions_ui, CellRegions *cell_regions, Universe *universe, UniversePosition mouse_universe_pos, b32 *mouse_click_consumed)
{
  if (ImGui::Begin("Cell Regions"))
  {
    if (universe != 0)
    {
      if (ImGui::Button("Make selection"))
      {
        cell_regions_ui->making_selection = true;
        cell_regions_ui->making_selection_dragging = false;
      }

      if (cell_regions_ui->making_selection)
      {
        ImGui::Text("Making selection");
      }

      if (!*mouse_click_consumed)
      {
        if (cell_regions_ui->making_selection)
        {
          *mouse_click_consumed = true;

          b32 update_end_position = false;
          if (ImGui::IsMouseClicked(0))
          {
            cell_regions_ui->making_selection_dragging = true;

            cell_regions_ui->start_selection_block = mouse_universe_pos.cell_block_position;
            cell_regions_ui->start_selection_cell = vec2_to_s32vec2(vec2_multiply(mouse_universe_pos.cell_position, universe->cell_block_dim));

            update_end_position = true;
          }

          if (cell_regions_ui->making_selection_dragging &&
              ImGui::IsMouseDragging(0))
          {
            ImGui::SameLine();
            ImGui::Text("Dragging");

            update_end_position = true;
          }

          if (update_end_position)
          {
            cell_regions_ui->end_selection_block = mouse_universe_pos.cell_block_position;
            cell_regions_ui->end_selection_cell = vec2_to_s32vec2(vec2_multiply(mouse_universe_pos.cell_position, universe->cell_block_dim));

            // Find direction of selection, so we can position end one past the mouse position in
            //   all directions

            s32vec2 block_delta = vec2_subtract(cell_regions_ui->end_selection_block, cell_regions_ui->start_selection_block);
            s32vec2 cell_delta = vec2_subtract(cell_regions_ui->end_selection_cell, cell_regions_ui->start_selection_cell);
            normalise_cell_coord(universe, &block_delta, &cell_delta);  // Moves all of the sign into the block coordinate

            s32vec2 zero_offset = vec2_max(vec2_sign(block_delta), {0, 0});

            cell_regions_ui->end_selection_cell = vec2_add(cell_regions_ui->end_selection_cell, zero_offset);
            normalise_cell_coord(universe, &cell_regions_ui->end_selection_block, &cell_regions_ui->end_selection_cell);
          }
        }

        if (!ImGui::IsMouseDown(0) && cell_regions_ui->making_selection_dragging)
        {
          cell_regions_ui->making_selection = false;
        }
      }

      ImGui::DragIntRange2("Selection block range x", &cell_regions_ui->start_selection_block.x, &cell_regions_ui->end_selection_block.x);
      ImGui::DragIntRange2("Selection block range y", &cell_regions_ui->start_selection_block.y, &cell_regions_ui->end_selection_block.y);

      ImGui::DragInt2("Start selection cell offset", cell_regions_ui->start_selection_cell.es, 1, 0, universe->cell_block_dim-1);
      ImGui::DragInt2("End selection cell offset", cell_regions_ui->end_selection_cell.es, 1, 0, universe->cell_block_dim-1);

      ImGui::InputText("New region name", cell_regions_ui->new_region_name_buffer, BUFFER_NAME_MAX_LENGTH);
      const char *give_region_name_label = "Give region a name first";
      if (ImGui::Button("Make new region"))
      {
        String name = new_string(cell_regions_ui->new_region_name_buffer);
        if (string_length(name) == 0)
        {
          ImGui::OpenPopup(give_region_name_label);
        }
        else
        {
          make_new_region(cell_regions, universe, cell_regions_ui->new_region_name_buffer, cell_regions_ui->start_selection_block, cell_regions_ui->start_selection_cell, cell_regions_ui->end_selection_block, cell_regions_ui->end_selection_cell);
        }
      }

      if (ImGui::BeginPopupModal(give_region_name_label))
      {
        if (ImGui::Button("OK"))
        {
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
    }

    for (u32 region_index = 0;
         region_index < cell_regions->regions.n_elements;
         ++region_index)
    {
      CellRegion& cell_region = cell_regions->regions[region_index];
      ImGui::Text("Name: %.*s", string_length(cell_region.name), cell_region.name.start);
    }
  }

  ImGui::End();
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
cell_region_selection_drawing_upload(CellRegionsUI *cell_regions_ui, Universe *universe, OpenGL_Buffer *general_universe_vbo, OpenGL_Buffer *general_universe_ibo)
{
  s32vec2 start_inner_corner_block = cell_regions_ui->start_selection_block;
  s32vec2 start_inner_corner_cell = cell_regions_ui->start_selection_cell;
  s32vec2 end_inner_corner_block = cell_regions_ui->end_selection_block;
  s32vec2 end_inner_corner_cell = cell_regions_ui->end_selection_cell;

  correct_cell_block_order(start_inner_corner_block, start_inner_corner_cell,
                           end_inner_corner_block, end_inner_corner_cell);

  vec2 start_inner_corner_cell_prop = vec2_divide((vec2){(r32)start_inner_corner_cell.x,  (r32)start_inner_corner_cell.y}, universe->cell_block_dim);
  vec2 end_inner_corner_cell_prop = vec2_divide((vec2){(r32)end_inner_corner_cell.x,  (r32)end_inner_corner_cell.y}, universe->cell_block_dim);

  // Outer border edge
  UniversePosition start_inner_corner = {start_inner_corner_block, start_inner_corner_cell_prop};
  UniversePosition end_inner_corner = {end_inner_corner_block, end_inner_corner_cell_prop};

  UniversePosition start_end_inner_corner, end_start_inner_corner;
  square_universe_coordinates(start_inner_corner, end_inner_corner, &start_end_inner_corner, &end_start_inner_corner);

  // Inner border edge
  UniversePosition start_outer_corner = {start_inner_corner.cell_block_position, vec2_add(start_inner_corner.cell_position, -0.05)};
  UniversePosition end_outer_corner = {end_inner_corner.cell_block_position, vec2_add(end_inner_corner.cell_position, 0.05)};

  UniversePosition start_end_outer_corner, end_start_outer_corner;
  square_universe_coordinates(start_outer_corner, end_outer_corner, &start_end_outer_corner, &end_start_outer_corner);

  vec4 inner_colour = {0.5, 0.5, 0.5, 1};
  GeneralUnvierseVertex vertex_inner_start     = {start_inner_corner, inner_colour};
  GeneralUnvierseVertex vertex_inner_start_end = {start_end_inner_corner, inner_colour};
  GeneralUnvierseVertex vertex_inner_end       = {end_inner_corner, inner_colour};
  GeneralUnvierseVertex vertex_inner_end_start = {end_start_inner_corner, inner_colour};

  vec4 outer_colour = {0.6, 0.6, 0.6, 1};
  GeneralUnvierseVertex vertex_outer_start     = {start_outer_corner, outer_colour};
  GeneralUnvierseVertex vertex_outer_start_end = {start_end_outer_corner, outer_colour};
  GeneralUnvierseVertex vertex_outer_end       = {end_outer_corner, outer_colour};
  GeneralUnvierseVertex vertex_outer_end_start = {end_start_outer_corner, outer_colour};

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
debug_cell_region_selection_drawing_upload(CellRegionsUI *cell_regions_ui, Universe *universe, OpenGL_Buffer *general_universe_vbo, OpenGL_Buffer *general_universe_ibo)
{
  UniversePosition start_corner = {cell_regions_ui->start_selection_block,
                                   vec2_divide((vec2){(r32)cell_regions_ui->start_selection_cell.x, (r32)cell_regions_ui->start_selection_cell.y}, universe->cell_block_dim)};
  UniversePosition end_corner = {cell_regions_ui->end_selection_block,
                                 vec2_divide((vec2){(r32)cell_regions_ui->end_selection_cell.x,  (r32)cell_regions_ui->end_selection_cell.y}, universe->cell_block_dim)};

  UniversePosition start_end_corner = {{start_corner.cell_block_position.x, end_corner.cell_block_position.y},
                                       {start_corner.cell_position.x, end_corner.cell_position.y}};
  UniversePosition end_start_corner = {{end_corner.cell_block_position.x, start_corner.cell_block_position.y},
                                       {end_corner.cell_position.x, start_corner.cell_position.y}};

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

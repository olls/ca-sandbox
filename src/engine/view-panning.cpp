#include "view-panning.h"

#include "maths.h"

#include "cell-block-coordinate-system.h"

#include "imgui.h"
#include "ccVector.h"

/// @file
///


void
update_view_projection_matrix(ViewPanning *view_panning, s32vec2 window_size)
{
  r32 aspect = (r32)window_size.y / window_size.x;
  mat4x4 matrix = {
    aspect * view_panning->scale, 0,                       0,  view_panning->scale*view_panning->offset.x,
    0,                           -1 * view_panning->scale, 0,  view_panning->scale*view_panning->offset.y,
    0,                            0,                       1,  0,
    0,                            0,                       0,  1
  };
  memcpy(view_panning->projection_matrix, matrix, sizeof(matrix));
}


void
update_view_panning(ViewPanning *view_panning, vec2 screen_mouse_pos)
{
  ImGuiIO& io = ImGui::GetIO();

  // TODO: Scaling acceleration could be more scientific
  const r32 scale_acceleration = 0.02;
  const r32 scale_deacceleration = 0.8;
  const r32 max_scale = 2.0;
  const r32 min_scale = 0.003;

  if (!io.WantCaptureMouse)
  {
    view_panning->scale_speed += io.MouseWheel * scale_acceleration;
    view_panning->scale *= 1 + view_panning->scale_speed;

    // TODO: Calculate pixel == cell scale for min?
    view_panning->scale = max(min_scale, min(max_scale, view_panning->scale));
    view_panning->scale_speed *= scale_deacceleration;
  }

  // Check mouse is in the window
  if (!(io.MousePos.x == -1 &&
        io.MousePos.y == -1))
  {
    vec2 d_mouse = vec2_subtract(screen_mouse_pos, view_panning->last_mouse_pos);
    view_panning->last_mouse_pos = screen_mouse_pos;

    // ImGui isn't using the mouse
    if (!io.WantCaptureMouse)
    {
      if (io.MouseDown[0])
      {
        vec2 scaled_mouse_pos = vec2_divide(d_mouse, view_panning->scale);
        view_panning->offset = vec2_add(view_panning->offset, scaled_mouse_pos);
      }
    }
  }
}


/// Inverses the cell block projection matrix to project a screen coordinate (from -1 -> 1) into a
///   UniversePosition
///
UniversePosition
screen_position_to_universe_position(ViewPanning *view_panning, vec2 screen_mouse_position)
{
  UniversePosition result;

  mat4x4 inverse_proj;
  mat4x4Inverse(inverse_proj, view_panning->projection_matrix);

  vec2 scaled_offset = vec2_multiply(view_panning->offset, view_panning->scale);
  vec2 offset_screen_mouse_position = vec2_subtract(screen_mouse_position, scaled_offset);

  vec4 offset_screen_mouse_position_4 = {offset_screen_mouse_position.x, offset_screen_mouse_position.y, 0, 1};
  vec4 cell_block_position_4 = mat4x4MultiplyVector(inverse_proj, offset_screen_mouse_position_4);
  vec2 cell_block_position = {cell_block_position_4.x, cell_block_position_4.y};

  result.cell_block_position = cell_block_round(cell_block_position);

  result.cell_position = vec2_subtract(cell_block_position, s32vec2_to_vec2(result.cell_block_position));

  return result;
}
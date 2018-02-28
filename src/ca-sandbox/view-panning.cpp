#include "ca-sandbox/view-panning.h"

#include "engine/maths.h"
#include "engine/vectors.h"

#include "ca-sandbox/cell-block-coordinate-system.h"

#include "imgui/imgui.h"

const s32 PANNING_MOUSE_BUTTON = 2;
const s32 PANNING_MOUSE_CTRL_BUTTON = 0;

/// @file
///


void
update_view_projection_matrix(ViewPanning *view_panning, mat4x4 aspect_ratio)
{
  mat4x4 matrix;
  mat4x4Identity(matrix);

  vec3 offset = {};
  offset.xy = view_panning->offset;
  mat4x4Translate(matrix, offset);

  mat4x4Scale(matrix, view_panning->scale);

  matrix[1][1] *= -1;

  mat4x4 matrix_scaled;
  mat4x4MultiplyMatrix(matrix_scaled, matrix, aspect_ratio);
  mat4x4Copy(view_panning->projection_matrix, matrix_scaled);
}


vec2
screen_position_to_cell_block_fraction(mat4x4 projection_matrix, vec2 screen_mouse_position)
{
  vec2 result;

  mat4x4 inverse_proj;
  mat4x4Inverse(inverse_proj, projection_matrix);

  vec4 screen_mouse_position_4 = {screen_mouse_position.x, screen_mouse_position.y, 0, 1};
  vec4 cell_block_position_4 = mat4x4MultiplyVector(inverse_proj, screen_mouse_position_4);
  result = {cell_block_position_4.x, cell_block_position_4.y};

  return result;
}


/// Inverses the cell block projection matrix to project a screen coordinate (from -1 -> 1) into a
///   UniversePosition
///
UniversePosition
screen_position_to_universe_position(ViewPanning *view_panning, vec2 screen_mouse_position)
{
  UniversePosition result;

  vec2 cell_block_position = screen_position_to_cell_block_fraction(view_panning->projection_matrix, screen_mouse_position);

  result.cell_block_position = cell_block_round(cell_block_position);
  result.cell_position = vec2_subtract(cell_block_position, s32vec2_to_vec2(result.cell_block_position));

  return result;
}


void
update_view_scaling(ViewPanning *view_panning, vec2 screen_mouse_pos)
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

    // TODO: Calculate pixel == cell scale for min?
  }

  view_panning->scale *= 1 + view_panning->scale_speed;
  view_panning->scale = max(min_scale, min(max_scale, view_panning->scale));
  view_panning->scale_speed *= scale_deacceleration;
}


void
update_view_panning(ViewPanning *view_panning, vec2 screen_mouse_pos, mat4x4 aspect_ratio, b32 *mouse_click_consumed)
{
  update_view_scaling(view_panning, screen_mouse_pos);

  view_panning->panning_last_frame = view_panning->currently_panning;

  // Mouse position difference between last frame and this frame
  vec2 d_mouse = vec2_subtract(screen_mouse_pos, view_panning->last_mouse_pos);
  view_panning->last_mouse_pos = screen_mouse_pos;

  ImGuiIO& io = ImGui::GetIO();

  if (!*mouse_click_consumed &&
      (ImGui::IsMouseClicked(PANNING_MOUSE_BUTTON) ||
       (io.KeyCtrl && ImGui::IsMouseClicked(PANNING_MOUSE_CTRL_BUTTON))))
  {
    view_panning->currently_panning = true;
  }

  if (view_panning->currently_panning &&
      !*mouse_click_consumed &&
      (ImGui::IsMouseDown(PANNING_MOUSE_BUTTON) ||
       (io.KeyCtrl && ImGui::IsMouseDown(PANNING_MOUSE_CTRL_BUTTON))))
  {
    *mouse_click_consumed = true;

    mat4x4 scale_matrix;
    mat4x4Copy(scale_matrix, aspect_ratio);
    mat4x4Scale(scale_matrix, view_panning->scale);
    mat4x4 inverse_scale;
    mat4x4Inverse(inverse_scale, scale_matrix);
    vec4 d_mouse_4 = {d_mouse.x, d_mouse.y, 0, 1};
    vec2 scaled_mouse_pos = mat4x4MultiplyVector(inverse_scale, d_mouse_4).xy;

    view_panning->offset = vec2_add(view_panning->offset, scaled_mouse_pos);

    view_panning->currently_panning = true;
  }
  else
  {
    view_panning->currently_panning = false;
  }
}
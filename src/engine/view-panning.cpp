#include "view-panning.h"

#include "maths.h"

#include "imgui.h"
#include "ccVector.h"


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
  const r32 scale_acceleration = 0.01;
  const r32 scale_deacceleration = 0.8;
  const r32 max_scale = 2.0;
  const r32 min_scale = 0.003;

  view_panning->scale_speed += io.MouseWheel * scale_acceleration;
  view_panning->scale *= 1 + view_panning->scale_speed;

  // TODO: Calculate pixel == cell scale for min?
  view_panning->scale = max(min_scale, min(max_scale, view_panning->scale));
  view_panning->scale_speed *= scale_deacceleration;

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
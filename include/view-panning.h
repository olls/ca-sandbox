#ifndef VIEW_PANNING
#define VIEW_PANNING

#include "types.h"
#include "vectors.h"

#include "ccVector.h"


struct ViewPanning
{
  r32 scale_speed;
  r32 scale;

  vec2 last_mouse_pos;
  vec2 offset;

  mat4x4 projection_matrix;
};


void
update_view_projection_matrix(ViewPanning *view_panning, s32vec2 window_size);


void
update_view_panning(ViewPanning *view_panning, vec2 screen_mouse_pos);


#endif
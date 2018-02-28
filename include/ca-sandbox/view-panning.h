#ifndef VIEW_PANNING
#define VIEW_PANNING

#include "engine/types.h"
#include "engine/vectors.h"

#include "ca-sandbox/cell-block-coordinate-system.h"

/// @file
///


struct ViewPanning
{
  r32 scale_speed;
  r32 scale;

  b32 currently_panning;

  /// Used to filter out mouse release events, if they were caused by the end of a pan.
  b32 panning_last_frame;

  vec2 last_mouse_pos;
  vec2 offset;

  mat4x4 projection_matrix;
};


void
update_view_projection_matrix(ViewPanning *view_panning, mat4x4 aspect_ratio);


void
update_view_panning(ViewPanning *view_panning, vec2 screen_mouse_pos, mat4x4 aspect_ratio, b32 *mouse_click_consumed);


UniversePosition
screen_position_to_universe_position(ViewPanning *view_panning, vec2 screen_mouse_position);


void
centre_universe(ViewPanning *view_panning, Universe *universe, s32vec2 window_size);


#endif
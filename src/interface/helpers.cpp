#include "interface/helpers.h"

#include "engine/vectors.h"

#include "imgui/imgui.h"


vec4
colour_for_background(vec4 background_colour)
{
  vec4 result;

  r32 hue;
  r32 saturation;
  r32 value;
  ImGui::ColorConvertRGBtoHSV(background_colour.x, background_colour.y, background_colour.z, hue, saturation, value);

  if (value > 0.4)
  {
    result = {0, 0, 0, 1};
  }
  else
  {
    result = {1, 1, 1, 1};
  }

  return result;
}
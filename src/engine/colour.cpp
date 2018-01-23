#include "engine/colour.h"

#include "engine/vectors.h"
#include "engine/maths.h"

#include "imgui/imgui.h"


vec4
lighten_colour(vec4 colour, r32 amount)
{
  vec4 result;

  result = vec4_add(colour, amount);
  result.x = min(max(result.x, 0.0f), 1.0f);
  result.y = min(max(result.y, 0.0f), 1.0f);
  result.z = min(max(result.z, 0.0f), 1.0f);
  result.w = colour.w;

  return result;
}


vec4
darken_colour(vec4 colour, r32 amount)
{
  return lighten_colour(colour, -amount);
}

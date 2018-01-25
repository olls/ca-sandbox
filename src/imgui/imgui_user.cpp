#include "imgui/imgui_user.h"

#include "engine/vectors.h"

#include "imgui/imgui.h"


b32
ImGui::RadioButton(const char* label, u32* v, u32 v_button)
{
  const b32 pressed = RadioButton(label, *v == v_button);
  if (pressed)
  {
    *v = v_button;
  }
  return pressed;
}


b32
ImGui::DragU32(const char* label, u32* v, r32 v_speed, u32 v_min, u32 v_max, const char* display_format)
{
    assert(*v <= MAX_S8);
    assert(v_min <= MAX_S8);
    assert(v_max <= MAX_S8);

    s32 v_f = (s32)*v;
    bool value_changed = DragInt(label, &v_f, v_speed, (s32)v_min, (s32)v_max, display_format);
    *v = (u32)v_f;
    return value_changed;
}


void
ImGui::Value(const char* prefix, vec2 vec)
{
  ImGui::Text("%s: [%f, %f]", prefix, vec.x, vec.y);
}


void
ImGui::Value(const char* prefix, s32vec2 vec)
{
  ImGui::Text("%s: [%d, %d]", prefix, vec.x, vec.y);
}


void
ImGui::Value(const char* prefix, uvec2 vec)
{
  ImGui::Text("%s: [%u, %u]", prefix, vec.x, vec.y);
}


void
ImGui::Value(const char* prefix, vec3 vec)
{
  ImGui::Text("%s: [%f, %f, %f]", prefix, vec.x, vec.y, vec.z);
}


void
ImGui::Value(const char* prefix, s32vec3 vec)
{
  ImGui::Text("%s: [%d, %d, %d]", prefix, vec.x, vec.y, vec.z);
}


void
ImGui::Value(const char* prefix, uvec3 vec)
{
  ImGui::Text("%s: [%u, %u, %u]", prefix, vec.x, vec.y, vec.z);
}


void
ImGui::Value(const char* prefix, vec4 vec)
{
  ImGui::Text("%s: [%f, %f, %f, %f]", prefix, vec.x, vec.y, vec.z, vec.w);
}


void
ImGui::Value(const char* prefix, s32vec4 vec)
{
  ImGui::Text("%s: [%d, %d, %d, %d]", prefix, vec.x, vec.y, vec.z, vec.w);
}


void
ImGui::Value(const char* prefix, uvec4 vec)
{
  ImGui::Text("%s: [%u, %u, %u, %u]", prefix, vec.x, vec.y, vec.z, vec.w);
}
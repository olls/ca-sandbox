#include "imgui/imgui_user.h"
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

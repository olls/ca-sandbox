#include "imgui_user.h"
#include "imgui.h"


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
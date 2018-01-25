#ifndef IMGUI_USER_H_DEF
#define IMGUI_USER_H_DEF

#include "engine/types.h"


namespace ImGui
{
  b32 RadioButton(const char *label, u32 *v, u32 v_button);
  b32 DragU32(const char* label, u32* v, r32 v_speed, u32 v_min, u32 v_max, const char* display_format);
};


#endif

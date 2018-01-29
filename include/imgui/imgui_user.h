#ifndef IMGUI_USER_H_DEF
#define IMGUI_USER_H_DEF

#include "engine/types.h"
#include "engine/vectors.h"

#include "imgui/imgui.h"


namespace ImGui
{
  bool BeginCustomMainMenuBar(ImVec2 start_offset, ImVec2 end_offset);
  b32 RadioButton(const char *label, u32 *v, u32 v_button);
  b32 DragU32(const char* label, u32* v, r32 v_speed, u32 v_min, u32 v_max, const char* display_format);
  void Value(const char* prefix, vec2 vec);
  void Value(const char* prefix, s32vec2 vec);
  void Value(const char* prefix, uvec2 vec);
  void Value(const char* prefix, vec3 vec);
  void Value(const char* prefix, s32vec3 vec);
  void Value(const char* prefix, uvec3 vec);
  void Value(const char* prefix, vec4 vec);
  void Value(const char* prefix, s32vec4 vec);
  void Value(const char* prefix, uvec4 vec);
};


#endif

#ifndef MAIN_GUI
#define MAIN_GUI

#include "ca-sandbox/ca-sandbox.h"


void
setup_imgui_style();


void
do_main_gui(CA_SandboxState *state, s32vec2 window_size, UniversePosition mouse_universe_position, b32 *mouse_click_consumed);


#endif
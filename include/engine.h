#ifndef ENGINE_H_DEF
#define ENGINE_H_DEF

#include "types.h"
#include "timing.h"

#include <SDL2/SDL.h>

/// @file
/// @brief  structs for managing the state of the engine
///


/// Stores the SDL_window and OpenGL context.
struct Engine
{
  SDL_Window *sdl_window;
  SDL_GLContext gl_context;
};


b32
start_engine(u32 argc, const char *argv[], const char window_name[], Engine *engine);


void
engine_swap_buffers(Engine *engine);


void
stop_engine(Engine *engine);


#endif
#ifndef ENGINE_H_DEF
#define ENGINE_H_DEF

#include "types.h"
#include "timing.h"

#include <SDL2/SDL.h>

/// @file
/// @brief  structs for managing the state of the engine (SDL window, frame timing, ...)
///


/// Stores the SDL_window and OpenGL context.
struct Window
{
  SDL_Window *sdl_window;
  SDL_GLContext gl_context;
};


/// Keeps track of state for the engine code, including frame timings.
struct Engine
{
  Window window;

  FPS_Counter fps;

  /// Number of micro-seconds per frame, this fixed and calculated from FPS.
  u32 useconds_per_frame;

  /// The number of micro-seconds the last frame took.
  u32 frame_dt;

  /// The time (in useconds) when engine_frame_start() was called.
  u64 frame_start;
};


b32
init_sdl(u32 argc, const char *argv[], const char window_name[], Engine *engine);

void
engine_setup_loop(Engine *engine);

void
engine_frame_start(Engine *engine);

void
engine_frame_end(Engine *engine);

void
stop_engine(Engine *engine);


#endif
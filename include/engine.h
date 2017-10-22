#ifndef ENGINE_H_DEF
#define ENGINE_H_DEF

#include <SDL2/SDL.h>

#include "types.h"
#include "timing.h"


struct SDL_Window
{
  u32 width;
  u32 height;

  SDL_Window *window;
  SDL_GLContext gl_context;
};


struct Engine
{
  SDL_Window sdl_window;

  FPS_Counter fps;

  u32 useconds_per_frame;
  u32 frame_dt;

  u64 frame_start;
};


b32
init_sdl(u32 argc, const char **argv, const char *name, Engine *engine);

void
engine_setup_loop(Engine *engine);

void
engine_frame_start(Engine *engine);

void
engine_frame_end(Engine *engine);

void
stop_engine(Engine *engine);


#endif
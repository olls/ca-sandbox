#ifndef ENGINE_H_DEF
#define ENGINE_H_DEF


#include <SDL2/SDL.h>

#include "types.h"


b32
start_engine(u32 argc, const char **argv, const char *name);

struct SDL_Window
{
  u32 width;
  u32 height;

  SDL_Window *window;
  SDL_GLContext gl_context;
};


#endif
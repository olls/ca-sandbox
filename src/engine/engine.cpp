#include "engine/engine.h"

#include "engine/types.h"
#include "engine/print.h"
#include "engine/opengl-util.h"

#include <SDL2/SDL.h>

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

/// @file
/// @brief  Contains functions for initialising SDL and OpenGL, as well as managing frame timings.
///


#ifdef _DEBUG
const b32 FULLSCREEN = false;
const u32 WINDOW_WIDTH = 1920;
const u32 WINDOW_HEIGHT = 1080;
#else
const b32 FULLSCREEN = false;
const u32 WINDOW_WIDTH = 1280;
const u32 WINDOW_HEIGHT = 720;
#endif


/// Initialises SDL and OpenGL, and creates the window.
b32
start_engine(u32 argc, const char *argv[], const char window_name[], Engine *engine)
{
  b32 success = true;

  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
      print("Failed to init SDL.\n");
      success = false;
      return success;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  SDL_WindowFlags flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL |
                                            SDL_WINDOW_SHOWN |
                                            SDL_WINDOW_MAXIMIZED |
                                            SDL_WINDOW_INPUT_FOCUS |
                                            SDL_WINDOW_RESIZABLE);
  u32 width = 0;
  u32 height = 0;
  if (FULLSCREEN)
  {
    flags = (SDL_WindowFlags)(flags | SDL_WINDOW_FULLSCREEN_DESKTOP);
  }
  else
  {
    width = WINDOW_WIDTH;
    height = WINDOW_HEIGHT;
  }

  engine->sdl_window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
  if (!engine->sdl_window)
  {
    print("Failed to initialise SDL window: %s\n", SDL_GetError());
    success = false;
    return success;
  }

  engine->gl_context = SDL_GL_CreateContext(engine->sdl_window);
  if (!engine->gl_context)
  {
    print("Failed to create OpenGL context.\n");
    success = false;
    return success;
  }

  SDL_GL_SetSwapInterval(1);

  glewExperimental = GL_TRUE;
  GLenum glew_status = glewInit();
  if (glew_status != GLEW_OK)
  {
    print("Failed to init GLEW: \"%s\"\n", glewGetErrorString(glew_status));
    success = false;
    return success;
  }

  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);

// #define OPEN_GL_DEBUG
#ifdef OPEN_GL_DEBUG
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(opengl_debug_output_callback, NULL);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif

  const unsigned char *opengl_version = glGetString(GL_VERSION);
  print("OpenGL Version: %s\n", opengl_version);
  opengl_print_errors();
  print("OpenGL init finished.\n");

  return success;
}


/// Swap the window buffers to update the screen
void
engine_swap_buffers(Engine *engine)
{
  SDL_GL_SwapWindow(engine->sdl_window);
}


/// De-initialises SDL
void
stop_engine(Engine *engine)
{
  SDL_GL_DeleteContext(engine->gl_context);
  SDL_DestroyWindow(engine->sdl_window);
  SDL_Quit();
}
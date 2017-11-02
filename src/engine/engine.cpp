#include <SDL2/SDL.h>

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>


#include "types.h"
#include "print.h"
#include "engine.h"
#include "timing.h"
#include "opengl-util.h"


/// @file
/// @brief Contains functions for initialising SDL and OpenGL, as well as managing frame timings.


const u32 FPS = 60;
const b32 FULLSCREEN = false;
const u32 WINDOW_WIDTH = 1280;
const u32 WINDOW_HEIGHT = 720;


/// Should be called before the main-loop starts. Sets up the frame timing.
void
engine_setup_loop(Engine *engine)
{
  engine->useconds_per_frame = 1000000 / FPS;
  engine->frame_dt = engine->useconds_per_frame;

  engine->fps.frame_count = 0;
  engine->fps.last_update = get_us();
}


/// Should be called at the start of a frame. Used to measure the frame time.
void
engine_frame_start(Engine *engine)
{
  engine->frame_start = get_us();
}


/// Should be called at the end of a frame. Swaps the SDL frame buffers, and maintains the FPS.
void
engine_frame_end(Engine *engine)
{
  SDL_GL_SwapWindow(engine->window.sdl_window);

  ++engine->fps.frame_count;
  if (engine->frame_start >= engine->fps.last_update + seconds_in_us(1))
  {
    engine->fps.last_update = engine->frame_start;
    engine->fps.current_avg = engine->fps.frame_count;
    engine->fps.frame_count = 0;
  }

  engine->frame_dt = get_us() - engine->frame_start;

  if (engine->frame_dt < engine->useconds_per_frame)
  {
    sleep_us(engine->useconds_per_frame - engine->frame_dt);
    engine->frame_dt = engine->useconds_per_frame;
  }
  else
  {
    print("Missed frame rate: %d\n", engine->frame_dt);
  }
}


/// Initialises SDL and OpenGL, and creates the window.
b32
init_sdl(u32 argc, const char *argv[], const char window_name[], Engine *engine)
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

  SDL_WindowFlags flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS);
  if (FULLSCREEN)
  {
    flags = (SDL_WindowFlags)(flags | SDL_WINDOW_FULLSCREEN_DESKTOP);
    engine->window.width = 0;
    engine->window.height = 0;
  }
  else
  {
    engine->window.width = WINDOW_WIDTH;
    engine->window.height = WINDOW_HEIGHT;
  }

  engine->window.sdl_window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, engine->window.width, engine->window.height, flags);
  if (!engine->window.sdl_window)
  {
    print("Failed to initialise SDL window: %s\n", SDL_GetError());
    success = false;
    return success;
  }

  if (FULLSCREEN)
  {
    s32 display_index = SDL_GetWindowDisplayIndex(engine->window.sdl_window);
    if (display_index < 0)
    {
      print("Failed to get display index.\n");
      success = false;
      return success;
    }
    SDL_Rect window_rect;
    if (SDL_GetDisplayBounds(display_index, &window_rect))
    {
      print("Failed to get display bounds.\n");
      success = false;
      return success;
    }
    engine->window.width = window_rect.w;
    engine->window.height = window_rect.h;
  }

  engine->window.gl_context = SDL_GL_CreateContext(engine->window.sdl_window);
  if (!engine->window.gl_context)
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

  glViewport(0, 0, engine->window.width, engine->window.height);

#define DEBUG
#ifdef DEBUG
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


/// De-initialises SDL
void
stop_engine(Engine *engine)
{
  SDL_GL_DeleteContext(engine->window.gl_context);
  SDL_DestroyWindow(engine->window.sdl_window);
  SDL_Quit();
}
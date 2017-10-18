#include <SDL2/SDL.h>

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>


#include "types.h"
#include "engine.h"
#include "timing.h"
#include "keys.h"
#include "gl-util.h"


const u32 FPS = 60;
const b32 FULLSCREEN = false;
const u32 WINDOW_WIDTH = 1920;
const u32 WINDOW_HEIGHT = 1080;


void
game_loop(SDL_Window *sdl_window, u32 argc, const char *argv[]/*, UpdateAndRenderFunc update_and_render_func*/)
{
  b32 running = true;

  Keys keys = {};

  u32 useconds_per_frame = 1000000 / FPS;
  u32 frame_dt = useconds_per_frame;

  FPS_Counter fps = {
    .frame_count = 0,
    .last_update = get_us()
  };

  while (running)
  {
    u64 last_frame_end = get_us();

    reset_key_events(&keys);

    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_QUIT:
        {
          running = false;
        } break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
          process_key_event(&keys, (SDL_KeyboardEvent *)&event);
        } break;

        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEWHEEL:
        {
        } break;
      }
    }

    // running &= update_and_render_func(renderer, &keys, &mouse, last_frame_end, frame_dt, fps.current_avg, argc, argv);

    if (keys.ascii[KEY_ESCAPE].down_event ||
        (keys.ascii['w'].down_event &&
         keys.ascii['w'].modifier | KMOD_CTRL) ||
        (keys.ascii['w'].down_event &&
         keys.ascii['w'].modifier | KMOD_CTRL))
    {
      running = false;
    }

    SDL_GL_SwapWindow(sdl_window->window);

    ++fps.frame_count;
    if (last_frame_end >= fps.last_update + seconds_in_us(1))
    {
      fps.last_update = last_frame_end;
      fps.current_avg = fps.frame_count;
      fps.frame_count = 0;
    }

    frame_dt = get_us() - last_frame_end;

    if (frame_dt < useconds_per_frame)
    {
      sleep_us(useconds_per_frame - frame_dt);
      frame_dt = useconds_per_frame;
    }
    else
    {
      printf("Missed frame rate: %d", frame_dt);
    }
  }
}


b32
start_engine(u32 argc, const char *argv[], const char window_name[])
{
  b32 success = true;

  SDL_Window sdl_window;

  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
      printf("Failed to init SDL.\n");
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
  }
  else
  {
    sdl_window.width = WINDOW_WIDTH;
    sdl_window.height = WINDOW_HEIGHT;
  }

  sdl_window.window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, sdl_window.width, sdl_window.height, flags);
  if (!sdl_window.window)
  {
    printf("Failed to initialise SDL window: %s\n", SDL_GetError());
    success = false;
    return success;
  }

  if (FULLSCREEN)
  {
    s32 display_index = SDL_GetWindowDisplayIndex(sdl_window.window);
    if (display_index < 0)
    {
      printf("Failed to get display index.\n");
      success = false;
      return success;
    }
    SDL_Rect window_rect;
    if (SDL_GetDisplayBounds(display_index, &window_rect))
    {
      printf("Failed to get display bounds.\n");
      success = false;
      return success;
    }
    sdl_window.width = window_rect.w;
    sdl_window.height = window_rect.h;
  }

  sdl_window.gl_context = SDL_GL_CreateContext(sdl_window.window);
  if (!sdl_window.gl_context)
  {
    printf("Failed to create OpenGL context.\n");
    success = false;
    return success;
  }

  SDL_GL_SetSwapInterval(1);

  glewExperimental = GL_TRUE;
  GLenum glew_status = glewInit();
  if (glew_status != GLEW_OK)
  {
    printf("Failed to init GLEW: \"%s\"\n", glewGetErrorString(glew_status));
    success = false;
    return success;
  }

  glViewport(0, 0, sdl_window.width, sdl_window.height);
  glEnable(GL_DEPTH_TEST);

#ifdef DEBUG
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(opengl_debug_output_callback, NULL);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif

  const unsigned char *opengl_version = glGetString(GL_VERSION);
  printf("OpenGL Version: %s\n", opengl_version);
  gl_print_errors();
  printf("OpenGL init finished.\n");

  game_loop(&sdl_window, argc, argv/*, update_and_render_func*/);

  SDL_GL_DeleteContext(sdl_window.gl_context);
  SDL_DestroyWindow(sdl_window.window);
  SDL_Quit();

  return success;
}
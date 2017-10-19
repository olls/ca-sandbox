#include <stdio.h>

#include "types.h"
#include "engine.h"


int
main(int argc, const char *argv[])
{
  Engine engine;
  b32 success = init_sdl(argc, argv, "Cellular Automata", &engine);

  engine_setup_loop(&engine);

  if (success)
  {
    b32 running = true;
    while (running)
    {
      running &= engine_loop_start(&engine);

      // running &= update_and_render_func(renderer, &keys, &mouse, last_frame_end, frame_dt, fps.current_avg, argc, argv);

      engine_loop_end(&engine);

    }
  }

  stop_engine(&engine);
  return success;
}

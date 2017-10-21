
#include "types.h"
#include "vectors.h"
#include "engine.h"
#include "cell-storage.h"


int
main(int argc, const char *argv[])
{
  Engine engine;
  b32 success = init_sdl(argc, argv, "Cellular Automata", &engine);

  engine_setup_loop(&engine);

  if (success)
  {
    Universe universe;

    b32 init = true;
    b32 running = true;
    while (running)
    {
      running &= engine_loop_start(&engine);

      if (init)
      {
        init = false;

        init_cell_hashmap(&universe);

        CellBlock *cell_block_a = get_cell_block(&universe, (s64Vec2){0, 0});
        CellBlock *cell_block_b = get_cell_block(&universe, (s64Vec2){0, 1});
        CellBlock *cell_block_c = get_cell_block(&universe, (s64Vec2){1, 0});
        CellBlock *cell_block_d = get_cell_block(&universe, (s64Vec2){1, 1});
      }

      engine_loop_end(&engine);
    }
  }

  stop_engine(&engine);
  return success;
}

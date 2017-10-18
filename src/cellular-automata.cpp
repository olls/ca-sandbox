#include <stdio.h>

#include "types.h"
#include "engine.h"


int
main(int argc, const char *argv[])
{
  b32 success = start_engine(argc, argv, "Cellular Automata");

  return success;
}

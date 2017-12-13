#include <stdlib.h>

#include "random.h"

#include "types.h"
#include "assert.h"


/// Returns a random unsigned 32 bit integer in min >= result > max.
u32
random_u32(u32 min, u32 max)
{
  // TODO: Something better than rand()...

  assert(max > min);

  u32 result = min + (rand() % (max - min));

  return result;
}
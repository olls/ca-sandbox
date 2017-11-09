#include <stdlib.h>

#include "allocate.h"

#include "types.h"


void *
_allocate(u32 size, u32 n)
{
  return malloc(size * n);
}
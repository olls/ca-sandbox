#include <stdlib.h>
#include <string.h>

#include "allocate.h"

#include "types.h"


void *
allocate_size(u32 size, u32 n)
{
  void *result = malloc(size * n);
  memset(result, 0, size * n);
  return result;
}


void *
re_allocate(void *pointer, u32 new_size)
{
  return realloc(pointer, new_size);
}


void
un_allocate(void *pointer)
{
  free(pointer);
}
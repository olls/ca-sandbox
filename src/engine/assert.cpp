#include <stdlib.h>

#include "types.h"
#include "print.h"
#include "assert.h"


/// @file
///


/// Assert wrapper which prints the file and line number of the assertion. (Using macro defined in
///   assert.h)
void
_assert(b32 expression_result, const char *filename, u32 line_number)
{
  if (!expression_result)
  {
    print("Assertion failed at %s:%d", filename, line_number);
    exit(EXIT_FAILURE);
  }
}
#include "assert.h"

#include "types.h"
#include "print.h"

#include <stdlib.h>

/// @file
///


/// Assert wrapper which prints the file and line number of the assertion. (Using macro defined in
///   assert.h)
void
_assert(b32 expression_result, const char *expression_text, const char *filename, u32 line_number)
{
  if (!expression_result)
  {
    print("Assertion (%s) failed at %s:%d", expression_text, filename, line_number);
    exit(EXIT_FAILURE);
  }
}
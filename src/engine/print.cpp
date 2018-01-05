#include "print.h"

#include <stdio.h>
#include <cstdarg>

/// @file
///


/// Custom printf wrapper.
void
print(const char format[], ...)
{
  va_list aptr;
  va_start(aptr, format);
  vprintf(format, aptr);
  va_end(aptr);
}
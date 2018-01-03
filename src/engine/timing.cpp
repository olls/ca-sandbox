#include "types.h"
#include "timing.h"

#include <sys/time.h>
#include <unistd.h>


/// @file


/// Returns the current time in micro-seconds
u64
get_us()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return ((u64)tv.tv_sec * (u64)1000000) + (u64)tv.tv_usec;
}


/// Sleeps for us micro-second
b32
sleep_us(u32 us)
{
  u32 error = usleep(us);
  return error;
}
#include <sys/time.h>
#include <unistd.h>

#include "types.h"
#include "timing.h"


u64
get_us()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return ((u64)tv.tv_sec * (u64)1000000) + (u64)tv.tv_usec;
}


b32
sleep_us(u32 us)
{
  u32 error = usleep(us);
  return error;
}
#include "timing.h"

#include "types.h"
#include "print.h"

#include <sys/time.h>
#include <unistd.h>

/// @file
///

const u32 FPS = 60;


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


/// Should be called before the main-loop starts. Sets up the frame timing.
void
engine_setup_loop(FrameTiming *frame_timing)
{
  frame_timing->useconds_per_frame = 1000000 / FPS;
  frame_timing->frame_dt = frame_timing->useconds_per_frame;

  frame_timing->fps.frame_count = 0;
  frame_timing->fps.last_update = get_us();
}


/// Should be called at the start of a frame. Used to measure the frame time.
void
engine_frame_start(FrameTiming *frame_timing)
{
  frame_timing->frame_start = get_us();
}


/// Should be called at the end of a frame. Swaps the SDL frame buffers, and maintains the FPS.
void
engine_frame_end(FrameTiming *frame_timing)
{
  ++frame_timing->fps.frame_count;
  if (frame_timing->frame_start >= frame_timing->fps.last_update + seconds_in_us(1))
  {
    frame_timing->fps.last_update = frame_timing->frame_start;
    frame_timing->fps.current_avg = frame_timing->fps.frame_count;
    frame_timing->fps.frame_count = 0;
  }

  frame_timing->frame_dt = get_us() - frame_timing->frame_start;

  if (frame_timing->frame_dt < frame_timing->useconds_per_frame)
  {
    sleep_us(frame_timing->useconds_per_frame - frame_timing->frame_dt);
    frame_timing->frame_dt = frame_timing->useconds_per_frame;
  }
  else
  {
    print("Missed frame rate: %d\n", frame_timing->frame_dt);
  }
}

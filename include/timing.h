#ifndef TIMING_H_DEF
#define TIMING_H_DEF


/// @file


#define seconds_in_ms(n) (1000 * (n))
#define seconds_in_us(n) (1000000 * (n))

#define ms_in_seconds(n) ((n) / (r32)1000.0)
#define us_in_seconds(n) ((n) / (r32)1000000.0)


u64
get_us();

b32
sleep_us(u32);


/// Used to keep a running average of the FPS
struct FPS_Counter
{
  /// Number of frames since the current_avg FPS was updated.
  u32 frame_count;

  /// Time in micro-seconds when the current_avg was updated.
  u64 last_update;

  /// Current average FPS.
  u32 current_avg;
};


#endif
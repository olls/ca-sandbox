#ifndef TIMING_H_DEF
#define TIMING_H_DEF

#include "types.h"

/// @file
/// @brief  Utilities for getting the current time, and recording FPS averages
///


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


/// Keeps track of state for the engine code, including frame timings.
struct FrameTiming
{
  FPS_Counter fps;

  /// Number of micro-seconds per frame, this is fixed and calculated from FPS.
  u32 useconds_per_frame;

  /// The number of micro-seconds the last frame took.
  u32 frame_dt;

  /// The time (in useconds) when engine_frame_start() was called.
  u64 frame_start;
};


void
engine_setup_loop(FrameTiming *frame_timing);


void
engine_frame_start(FrameTiming *frame_timing);


void
engine_frame_end(FrameTiming *frame_timing);


#endif
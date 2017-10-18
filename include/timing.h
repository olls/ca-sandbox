#ifndef TIMING_H_DEF
#define TIMING_H_DEF


#define seconds_in_ms(n) (1000 * (n))
#define seconds_in_us(n) (1000000 * (n))

#define ms_in_seconds(n) ((n) / (r32)1000.0)
#define us_in_seconds(n) ((n) / (r32)1000000.0)


u64
get_us();

b32
sleep_us(u32);


struct FPS_Counter
{
  u32 frame_count;
  u64 last_update;
  u32 current_avg;
};


#endif
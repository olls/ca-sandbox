#ifndef HUMAN_TIME_H_DEF
#define HUMAN_TIME_H_DEF

const u32 ONE_US   = 1;
const u32 ONE_MS   = ONE_US * 1000;
const u32 ONE_S    = ONE_MS * 1000;
const u32 ONE_MIN  = ONE_S * 60;;
const u32 ONE_HOUR = ONE_MIN * 60;


inline r32
human_time(u32 us, const char **unit_result)
{
  r32 result = us;
  *unit_result = "us";

  if (us > ONE_HOUR)
  {
    result = us * (1.0/ONE_HOUR);
    *unit_result = "hours";
  }
  else if (us > ONE_MIN)
  {
    result = us * (1.0/ONE_MIN);
    *unit_result = "minuets";
  }
  else if (us > ONE_S)
  {
    result = us * (1.0/ONE_S);
    *unit_result = "seconds";
  }
  else if (us > ONE_MS)
  {
    result = us * (1.0/ONE_MS);
    *unit_result = "ms";
  }

  return result;
}


#endif
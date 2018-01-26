#ifndef MATHS_H_DEF
#define MATHS_H_DEF

#include <math.h>

/// @file
/// @brief  Templates for basic maths functions
///


template <typename T>
T
max(T a, T b)
{
  T result;

  if (a > b)
  {
    result = a;
  }
  else
  {
    result = b;
  }

  return result;
}


template <typename T>
T
min(T a, T b)
{
  T result;

  if (a < b)
  {
    result = a;
  }
  else
  {
    result = b;
  }

  return result;
}


template <typename T>
T
clamp(T lower_bound, T upper_bound, T v)
{
  T result = v;

  result = min(upper_bound, result);
  result = max(lower_bound, result);

  return result;
}


template <typename T>
void
clamp(T lower_bound, T upper_bound, T *v)
{
  *v = min(upper_bound, *v);
  *v = max(lower_bound, *v);
}


inline u64
ipow(u64 base, u64 exp)
{
    u64 result = 1;
    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }

    return result;
}


template <typename T>
T
sign(T x)
{
  return x >= 0 ? 1 : -1;
}


#endif
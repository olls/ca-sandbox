#ifndef MATHS_H_DEF
#define MATHS_H_DEF


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


#endif
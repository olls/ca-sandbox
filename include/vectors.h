#ifndef VECTORS_H_DEF
#define VECTORS_H_DEF

#include "types.h"


struct Vec2
{
  r32 x;
  r32 y;
};


struct uVec2
{
  u32 x;
  u32 y;
};


struct sVec2
{
  s32 x;
  s32 y;
};


struct s64Vec2
{
  s64 x;
  s64 y;
};


template <typename Vec>
Vec
operator+(Vec a, Vec b)
{
  Vec result;
  result.x = a.x + b.x;
  result.y = a.y + b.y;
  return result;
}

template <typename Vec>
Vec
operator-(Vec a, Vec b)
{
  Vec result;
  result.x = a.x - b.x;
  result.y = a.y - b.y;
  return result;
}

template <typename Vec>
Vec
operator*(Vec a, Vec b)
{
  Vec result;
  result.x = a.x * b.x;
  result.y = a.y * b.y;
  return result;
}

template <typename Vec>
Vec
operator/(Vec a, Vec b)
{
  Vec result;
  result.x = a.x / b.x;
  result.y = a.y / b.y;
  return result;
}

template <typename Vec, typename Scalar>
Vec
operator+(Vec a, Scalar b)
{
  Vec result;
  result.x = a.x + b;
  result.y = a.y + b;
  return result;
}

template <typename Vec, typename Scalar>
Vec
operator-(Vec a, Scalar b)
{
  Vec result;
  result.x = a.x - b;
  result.y = a.y - b;
  return result;
}

template <typename Vec, typename Scalar>
Vec
operator*(Vec a, Scalar b)
{
  Vec result;
  result.x = a.x * b;
  result.y = a.y * b;
  return result;
}

template <typename Vec, typename Scalar>
Vec
operator/(Vec a, Scalar b)
{
  Vec result;
  result.x = a.x / b;
  result.y = a.y / b;
  return result;
}

template <typename Vec>
b32
operator==(Vec a, Vec b)
{
  b32 result = a.x == b.x &&
               a.y == b.y;
  return result;
}

template <typename Vec>
b32
operator!=(Vec a, Vec b)
{
  b32 result = !(a == b);
  return result;
}


#endif
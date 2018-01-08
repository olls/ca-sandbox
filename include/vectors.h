#ifndef VECTORS_H_DEF
#define VECTORS_H_DEF

#include "types.h"

#include "ccVector.h"

/// @file
/// @brief Vector structs and templates for operating on them
///
/// ccVector provides vec2, vec3, vec4.  This file adds integer vector types, and provides templated
///   vector functions


struct uvec2
{
  union
  {
    struct
    {
      u32 x;
      u32 y;
    };
    struct
    {
      u32 es[2];
    };
  };
};


struct s32vec2
{
  union
  {
    struct
    {
      s32 x;
      s32 y;
    };
    struct
    {
      s32 es[2];
    };
  };
};


struct s64vec2
{
  union
  {
    struct
    {
      s64 x;
      s64 y;
    };
    struct
    {
      s64 es[2];
    };
  };
};


/// Truncate u32 vector values to r32 vector.
inline vec2
uvec2_to_vec2(uvec2 a)
{
  vec2 result = {(r32)a.x, (r32)a.y};
  return result;
}

/// Truncate r32 vector values to s32 vector.
inline s32vec2
vec2_to_s32vec2(vec2 a)
{
  s32vec2 result = {(s32)a.x, (s32)a.y};
  return result;
}

/// Truncate s32 vector values to r32 vector.
inline vec2
s32vec2_to_vec2(s32vec2 a)
{
  vec2 result = {(r32)a.x, (r32)a.y};
  return result;
}

template <typename vec>
inline vec
vec2_add(vec a, vec b)
{
  vec result;
  result.x = a.x + b.x;
  result.y = a.y + b.y;
  return result;
}

template <typename vec>
inline vec
vec2_subtract(vec a, vec b)
{
  vec result;
  result.x = a.x - b.x;
  result.y = a.y - b.y;
  return result;
}

template <typename vec>
inline vec
vec2_multiply(vec a, vec b)
{
  vec result;
  result.x = a.x * b.x;
  result.y = a.y * b.y;
  return result;
}

template <typename vec>
inline vec
vec2_divide(vec a, vec b)
{
  vec result;
  result.x = a.x / b.x;
  result.y = a.y / b.y;
  return result;
}

template <typename vec, typename Scalar>
inline vec
vec2_add(vec a, Scalar b)
{
  vec result;
  result.x = a.x + b;
  result.y = a.y + b;
  return result;
}

template <typename vec, typename Scalar>
inline vec
vec2_subtract(vec a, Scalar b)
{
  vec result;
  result.x = a.x - b;
  result.y = a.y - b;
  return result;
}

template <typename vec, typename Scalar>
inline vec
vec2_multiply(vec a, Scalar b)
{
  vec result;
  result.x = a.x * b;
  result.y = a.y * b;
  return result;
}

template <typename vec, typename Scalar>
inline vec
vec2_divide(vec a, Scalar b)
{
  vec result;
  result.x = a.x / b;
  result.y = a.y / b;
  return result;
}

template <typename vec>
inline b32
vec2_eq(vec a, vec b)
{
  b32 result = a.x == b.x &&
               a.y == b.y;
  return result;
}


struct uvec3
{
  union
  {
    struct
    {
      u32 x;
      u32 y;
      u32 z;
    };
    struct
    {
      u32 es[3];
    };
  };
};


struct s32vec3
{
  union
  {
    struct
    {
      s32 x;
      s32 y;
      s32 z;
    };
    struct
    {
      s32 es[3];
    };
  };
};


struct s64vec3
{
  union
  {
    struct
    {
      s64 x;
      s64 y;
      s64 z;
    };
    struct
    {
      s64 es[3];
    };
  };
};


/// Truncate u32 vector values to r32 vector.
inline vec3
uvec3_to_vec3(uvec3 a)
{
  vec3 result = {(r32)a.x, (r32)a.y, (r32)a.z};
  return result;
}

/// Truncate r32 vector values to s32 vector.
inline s32vec3
vec3_to_s32vec3(vec3 a)
{
  s32vec3 result = {(s32)a.x, (s32)a.y, (s32)a.z};
  return result;
}

/// Truncate s32 vector values to r32 vector.
inline vec3
s32vec3_to_vec3(s32vec3 a)
{
  vec3 result = {(r32)a.x, (r32)a.y, (r32)a.z};
  return result;
}

template <typename vec>
inline vec
vec3_add(vec a, vec b)
{
  vec result;
  result.x = a.x + b.x;
  result.y = a.y + b.y;
  result.z = a.z + b.z;
  return result;
}

template <typename vec>
inline vec
vec3_subtract(vec a, vec b)
{
  vec result;
  result.x = a.x - b.x;
  result.y = a.y - b.y;
  result.z = a.z - b.z;
  return result;
}

template <typename vec>
inline vec
vec3_multiply(vec a, vec b)
{
  vec result;
  result.x = a.x * b.x;
  result.y = a.y * b.y;
  result.z = a.z * b.z;
  return result;
}

template <typename vec>
inline vec
vec3_divide(vec a, vec b)
{
  vec result;
  result.x = a.x / b.x;
  result.y = a.y / b.y;
  result.z = a.z / b.z;
  return result;
}

template <typename vec, typename Scalar>
inline vec
vec3_add(vec a, Scalar b)
{
  vec result;
  result.x = a.x + b;
  result.y = a.y + b;
  result.z = a.z + b;
  return result;
}

template <typename vec, typename Scalar>
inline vec
vec3_subtract(vec a, Scalar b)
{
  vec result;
  result.x = a.x - b;
  result.y = a.y - b;
  result.z = a.z - b;
  return result;
}

template <typename vec, typename Scalar>
inline vec
vec3_multiply(vec a, Scalar b)
{
  vec result;
  result.x = a.x * b;
  result.y = a.y * b;
  result.z = a.z * b;
  return result;
}

template <typename vec, typename Scalar>
inline vec
vec3_divide(vec a, Scalar b)
{
  vec result;
  result.x = a.x / b;
  result.y = a.y / b;
  result.z = a.z / b;
  return result;
}

template <typename vec>
inline b32
vec3_eq(vec a, vec b)
{
  b32 result = a.x == b.x &&
               a.y == b.y &&
               a.z == b.z;
  return result;
}


struct uvec4
{
  union
  {
    struct
    {
      u32 x;
      u32 y;
      u32 z;
      u32 w;
    };
    struct
    {
      u32 es[4];
    };
  };
};


struct s32vec4
{
  union
  {
    struct
    {
      s32 x;
      s32 y;
      s32 z;
      s32 w;
    };
    struct
    {
      s32 es[4];
    };
  };
};


struct s64vec4
{
  union
  {
    struct
    {
      s64 x;
      s64 y;
      s64 z;
      s64 w;
    };
    struct
    {
      s64 es[4];
    };
  };
};


/// Truncate u32 vector values to r32 vector.
inline vec4
uvec4_to_vec4(uvec4 a)
{
  vec4 result = {(r32)a.x, (r32)a.y, (r32)a.z, (r32)a.w};
  return result;
}

/// Truncate r32 vector values to s32 vector.
inline s32vec4
vec4_to_s32vec4(vec4 a)
{
  s32vec4 result = {(s32)a.x, (s32)a.y, (s32)a.z, (s32)a.w};
  return result;
}

/// Truncate s32 vector values to r32 vector.
inline vec4
s32vec4_to_vec4(s32vec4 a)
{
  vec4 result = {(r32)a.x, (r32)a.y, (r32)a.z, (r32)a.w};
  return result;
}

template <typename vec>
inline vec
vec4_add(vec a, vec b)
{
  vec result;
  result.x = a.x + b.x;
  result.y = a.y + b.y;
  result.z = a.z + b.z;
  result.w = a.w + b.w;
  return result;
}

template <typename vec>
inline vec
vec4_subtract(vec a, vec b)
{
  vec result;
  result.x = a.x - b.x;
  result.y = a.y - b.y;
  result.z = a.z - b.z;
  result.w = a.w - b.w;
  return result;
}

template <typename vec>
inline vec
vec4_multiply(vec a, vec b)
{
  vec result;
  result.x = a.x * b.x;
  result.y = a.y * b.y;
  result.z = a.z * b.z;
  result.w = a.w * b.w;
  return result;
}

template <typename vec>
inline vec
vec4_divide(vec a, vec b)
{
  vec result;
  result.x = a.x / b.x;
  result.y = a.y / b.y;
  result.z = a.z / b.z;
  result.w = a.w / b.w;
  return result;
}

template <typename vec, typename Scalar>
inline vec
vec4_add(vec a, Scalar b)
{
  vec result;
  result.x = a.x + b;
  result.y = a.y + b;
  result.z = a.z + b;
  result.w = a.w + b;
  return result;
}

template <typename vec, typename Scalar>
inline vec
vec4_subtract(vec a, Scalar b)
{
  vec result;
  result.x = a.x - b;
  result.y = a.y - b;
  result.z = a.z - b;
  result.w = a.w - b;
  return result;
}

template <typename vec, typename Scalar>
inline vec
vec4_multiply(vec a, Scalar b)
{
  vec result;
  result.x = a.x * b;
  result.y = a.y * b;
  result.z = a.z * b;
  result.w = a.w * b;
  return result;
}

template <typename vec, typename Scalar>
inline vec
vec4_divide(vec a, Scalar b)
{
  vec result;
  result.x = a.x / b;
  result.y = a.y / b;
  result.z = a.z / b;
  result.w = a.w / b;
  return result;
}

template <typename vec>
inline b32
vec4_eq(vec a, vec b)
{
  b32 result = a.x == b.x &&
               a.y == b.y &&
               a.z == b.z &&
               a.w == b.w;
  return result;
}


#endif
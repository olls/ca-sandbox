#ifndef VECTORS_H_DEF
#define VECTORS_H_DEF

#include "types.h"


struct vec2
{
  r32 x;
  r32 y;
};


struct uvec2
{
  u32 x;
  u32 y;
};


struct s32vec2
{
  s32 x;
  s32 y;
};


struct s64vec2
{
  s64 x;
  s64 y;
};


inline vec2
uvec2_to_vec2(uvec2 a)
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


struct vec3
{
  r32 x;
  r32 y;
  r32 z;
};


struct uvec3
{
  u32 x;
  u32 y;
  u32 z;
};


struct s32vec3
{
  s32 x;
  s32 y;
  s32 z;
};


struct s64vec3
{
  s64 x;
  s64 y;
  s64 z;
};


inline vec3
uvec3_to_vec3(uvec3 a)
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


struct vec4
{
  r32 w;
  r32 x;
  r32 y;
  r32 z;
};


struct uvec4
{
  u32 w;
  u32 x;
  u32 y;
  u32 z;
};


struct s32vec4
{
  s32 w;
  s32 x;
  s32 y;
  s32 z;
};


struct s64vec4
{
  s64 w;
  s64 x;
  s64 y;
  s64 z;
};


inline vec4
uvec4_to_vec4(uvec4 a)
{
  vec4 result = {(r32)a.w, (r32)a.x, (r32)a.y, (r32)a.z};
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
  return result;
}

template <typename vec>
inline b32
vec4_eq(vec a, vec b)
{
  b32 result = a.x == b.x &&
               a.y == b.y &&
               a.z == b.z;
  return result;
}


#endif
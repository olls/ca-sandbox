#ifndef TYPES_H_DEF
#define TYPES_H_DEF

#include <stdint.h>

/// @file
/// @brief  Custom typedefs and type bounds.
///


typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

typedef float r32;
typedef double r64;

typedef u32 b32;


const u32 MIN_U8  = 0;
const u8  MAX_U8  = UINT8_MAX;

const u32 MIN_U16 = 0;
const u16 MAX_U16 = UINT16_MAX;

const u32 MIN_U32 = 0;
const u32 MAX_U32 = UINT32_MAX;

const u32 MIN_U64 = 0;
const u64 MAX_U64 = UINT64_MAX;

const s8  MIN_S8  = INT8_MIN;
const s8  MAX_S8  = INT8_MAX;

const s16 MIN_S16 = INT16_MIN;
const s16 MAX_S16 = INT16_MAX;

const s32 MIN_S32 = INT32_MIN;
const s32 MAX_S32 = INT32_MAX;

const s64 MIN_S64 = INT64_MIN;
const s64 MAX_S64 = INT64_MAX;


#endif
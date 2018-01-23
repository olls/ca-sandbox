#ifndef MY_ARRAY_H_DEF
#define MY_ARRAY_H_DEF

/// @file
///
/// Wrapper for array.h defining custom allocator and assertion procedures.
///


#include "engine/allocate.h"
#include "engine/assert.h"

#define _ARRAY_ALLOCATE_FUNC(size) (allocate_size((size), 1))
#define _ARRAY_FREE_FUNC(p) (un_allocate(p))
#define _ARRAY_ASSERT_FUNC assert

#include "header-libs/array.h"


#endif
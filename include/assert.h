#ifndef ASSERT_H_DEF
#define ASSERT_H_DEF

#include "types.h"


#define assert(expression_result) _assert((expression_result), (__FILE__), (__LINE__))


void
_assert(b32 expression_result, const char *filename, u32 line_number);


#endif
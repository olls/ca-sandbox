#ifndef ASSERT_H_DEF
#define ASSERT_H_DEF

#include "types.h"


#define _STRING(x) #x
#define assert(expression_result) _assert((expression_result), _STRING(expression_result), (__FILE__), (__LINE__))


void
_assert(b32 expression_result, const char *expression_text, const char *filename, u32 line_number);


#endif
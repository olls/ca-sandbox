#ifndef COMPARISON_OPERATOR_H_DEF
#define COMPARISON_OPERATOR_H_DEF

#include "types.h"
#include "text.h"


enum struct ComparisonOp : u32
{
  GREATER_THAN,
  GREATER_THAN_EQUAL,
  EQUAL,
  LESS_THAN_EQUAL,
  LESS_THAN
};


static const char *COMPARISON_OPERATOR_STRINGS[] = {
  ">",
  ">=",
  "=",
  "<=",
  "<"
};


#endif
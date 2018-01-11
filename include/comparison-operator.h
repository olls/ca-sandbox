#ifndef COMPARISON_OPERATOR_H_DEF
#define COMPARISON_OPERATOR_H_DEF

#include "types.h"
#include "text.h"


enum struct ComparisonOp : u32
{
  GREATER_THAN,
  LESS_THAN,
  EQUALS
};


static const char *COMPARISON_OPERATOR_STRINGS[] = {
  ">",
  "<",
  "="
};


#endif
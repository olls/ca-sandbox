#ifndef LOAD_RULE_H_DEF
#define LOAD_RULE_H_DEF

#include "types.h"
#include "extendable-array.h"

#include "cell.h"


struct CellStateWildcard
{
  b32 wildcard;
  CellState state;
};


struct RulePattern
{
  CellState result;
  CellStateWildcard cell_states[];
};


b32
load_rule_file(const char *filename, ExtendableArray *rule_patterns);


#endif
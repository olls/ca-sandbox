#ifndef EXTENDABLE_ARRAY_H_DEF
#define EXTENDABLE_ARRAY_H_DEF

#include "types.h"


const u32 EXTENDABLE_ARRAY_INITIAL_SIZE = 16;


struct ExtendableArray
{
  u32 element_size;
  u32 array_size;
  u32 next_free_element_position;
  void *elements;
};


void
new_extendable_array(u32 element_size, ExtendableArray *result);


void *
get_from_extendable_array(ExtendableArray *array, u32 position);


u32
add_to_extendable_array(ExtendableArray *array, void *new_element);


#endif
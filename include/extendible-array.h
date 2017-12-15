#ifndef EXTENDIBLE_ARRAY_H_DEF
#define EXTENDIBLE_ARRAY_H_DEF

#include "types.h"


const u32 EXTENDIBLE_ARRAY_INITIAL_SIZE = 16;


struct ExtendibleArray
{
  u32 element_size;
  u32 array_size;
  u32 next_free_element_position;
  void *elements;
};


void
new_extendible_array(u32 element_size, ExtendibleArray *result);


void *
get_from_extendible_array(ExtendibleArray *array, u32 position);


u32
add_to_extendible_array(ExtendibleArray *array, void *new_element);


#endif
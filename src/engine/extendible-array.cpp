#include <string.h>

#include "extendible-array.h"

#include "allocate.h"
#include "assert.h"


void
new_extendible_array(u32 element_size, ExtendibleArray *result)
{
  result->element_size = element_size,
  result->array_size = EXTENDIBLE_ARRAY_INITIAL_SIZE,
  result->next_free_element_position = 0,

  result->elements = allocate_size(result->element_size, result->array_size);
}


void *
get_from_extendible_array(ExtendibleArray *array, u32 position)
{
  void *result;

  result = (void *)((u8 *)array->elements + (array->element_size * position));

  return result;
}


u32
add_to_extendible_array(ExtendibleArray *array, void *new_element)
{
  if (array->next_free_element_position == array->array_size)
  {
    array->array_size *= 2;
    array->elements = re_allocate(array->elements, array->element_size * array->array_size);
  }

  assert(array->elements != 0);

  u32 position = array->next_free_element_position;
  ++array->next_free_element_position;

  void *slot = get_from_extendible_array(array, position);
  memcpy(slot, new_element, array->element_size);

  return position;
}
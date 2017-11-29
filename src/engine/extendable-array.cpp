#include <string.h>

#include "extendable-array.h"

#include "allocate.h"
#include "assert.h"


void
new_extendable_array(u32 element_size, ExtendableArray *result)
{
  result->element_size = element_size,
  result->array_size = EXTENDABLE_ARRAY_INITIAL_SIZE,
  result->next_free_element_position = 0,

  result->elements = allocate_size(result->element_size, result->array_size);
}


void *
get_from_extendable_array(ExtendableArray *array, u32 position)
{
  void *result;

  result = (void *)((u8 *)array->elements + (array->element_size * position));

  return result;
}


u32
add_to_extendable_array(ExtendableArray *array, void *new_element)
{
  if (array->next_free_element_position == array->array_size)
  {
    array->array_size *= 2;
    array->elements = re_allocate(array->elements, array->element_size * array->array_size);
  }

  assert(array->elements != 0);

  u32 position = array->next_free_element_position;
  ++array->next_free_element_position;

  void *slot = get_from_extendable_array(array, position);
  memcpy(slot, new_element, array->element_size);

  return position;
}
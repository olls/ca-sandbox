#ifndef EXTENDABLE_ARRAY_H_DEF
#define EXTENDABLE_ARRAY_H_DEF

#include "types.h"
#include "allocate.h"
#include "assert.h"

#define EXTENDABLE_ARRAY_INITIAL_SIZE 16


/// ExtendableArray is a templated dynamic continuous array allocated on the heap.
///
/// - The templatation allows you to specify a type T to be stored in the array.
/// - You can also provide an element size into the constructor, which enables un-typed use, or
///     using flexible array members for dynamicly sized structs.
///   - By default element_size is set to sizeof(T)
///
template <typename T>
struct ExtendableArray
{
  u32 element_size;
  u32 array_size;
  union
  {
    u32 n_elements;
    u32 next_free_element_position;
  };

  T *elements;

  void
  allocate_array(u32 _element_size=sizeof(T));

  void
  un_allocate_array();

  void
  clear_array();

  T *
  get(u32 position);

  u32
  add(T *new_element);

  u32
  add(T new_element);

  u32
  add(u32 n_new_elements, T *new_elements);

  T *
  get_new_element();

  void
  remove(T element);
};


// Member functions must be defined in the header file, as the templates cannot be compiled until
//   they are instantiated.


template <typename T>
void
ExtendableArray<T>::allocate_array(u32 _element_size)
{
  this->element_size = _element_size;
  this->array_size = EXTENDABLE_ARRAY_INITIAL_SIZE;
  this->elements = (T *)allocate_size(this->element_size, this->array_size);
  this->n_elements = 0;
}


template <typename T>
void
ExtendableArray<T>::un_allocate_array()
{
  if (this->elements != 0)
  {
    un_allocate(this->elements);
  }

  this->elements = 0;
  this->element_size = 0;
  this->n_elements = 0;
  this->array_size = 0;
}


template <typename T>
void
ExtendableArray<T>::clear_array()
{
  this->n_elements = 0;
}


template <typename T>
T *
ExtendableArray<T>::get(u32 position)
{
  T *result;
  result = (T *)((u8 *)this->elements + (this->element_size * position));
  return result;
}


template <typename T>
u32
ExtendableArray<T>::add(T *new_element)
{
  if (this->next_free_element_position >= this->array_size)
  {
    this->array_size *= 2;
    this->elements = (T *)re_allocate(this->elements, this->element_size * this->array_size);
  }

  assert(this->elements != 0);

  u32 position = this->next_free_element_position;
  ++this->next_free_element_position;

  T *slot = get(position);

  if (new_element != 0)
  {
    // Copy in new element
    memcpy(slot, new_element, this->element_size);
  }
  else
  {
    // Clear slot
    memset(slot, 0, this->element_size);
  }

  return position;
}


template <typename T>
u32
ExtendableArray<T>::add(T new_element)
{
  return this->add(&new_element);
}


template <typename T>
u32
ExtendableArray<T>::add(u32 n_new_elements, T *new_elements)
{
  if (this->array_size < this->next_free_element_position + n_new_elements)
  {
    while (this->array_size < this->next_free_element_position + n_new_elements)
    {
      this->array_size *= 2;
    }

    this->elements = (T *)re_allocate(this->elements, this->element_size * this->array_size);
  }

  assert(this->elements != 0);

  u32 position = this->next_free_element_position;
  this->next_free_element_position += n_new_elements;

  T *start_position = this->get(position);
  memcpy(start_position, new_elements, this->element_size * n_new_elements);

  return position;
}


template <typename T>
T *
ExtendableArray<T>::get_new_element()
{
  u32 new_position = this->add(0);
  T *result = this->get(new_position);
  return result;
}


template <typename T>
void
ExtendableArray<T>::remove(T element)
{
  for (u32 element_n = 0;
       element_n < this->n_elements;
       ++element_n)
  {
    T *test_element = this->get(element_n);
    if (*test_element == element)
    {
      // Copy the last element into this slot

      u32 element_to_remove_n = element_n;
      u32 element_to_move_n = this->n_elements;

      T *element_to_remove = this->get(element_to_remove_n);
      T *element_to_move = this->get(element_to_move_n);

      *element_to_remove = *element_to_move;

      this->n_elements -= 1;
    }
  }
}


#endif

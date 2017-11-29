#ifndef ALLOCATE_H_DEF
#define ALLOCATE_H_DEF

#include "types.h"


#define allocate(type, n) ((type *)allocate_size(sizeof(type), (n)))


void *
allocate_size(u32 size, u32 n);


void *
re_allocate(void *pointer, u32 new_size);


void
un_allocate(void *pointer);


#endif
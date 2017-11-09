#ifndef ALLOCATE_H_DEF
#define ALLOCATE_H_DEF

#include "types.h"


#define allocate(type, n) ((type *)_allocate(sizeof(type), (n)))


void *
_allocate(u32 size, u32 n);


#endif
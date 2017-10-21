#ifndef FILES_H_DEF
#define FILES_H_DEF

#include "types.h"
#include "files.h"


struct File
{
  s32 fd;
  const char *read_ptr;
  u8 *write_ptr;
  s32 size;
};


b32
open_file(const char *filename, File *result, b32 write = false, s32 trunc_to = -1);


b32
close_file(File *file, s32 trunc_to = -1);


#endif
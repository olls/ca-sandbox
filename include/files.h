#ifndef FILES_H_DEF
#define FILES_H_DEF

#include "types.h"


/// @file
///


/// Object used to hold a file reference for a `mmap`-ed file using open_file()
struct File
{
  /// The OS file descriptor
  s32 fd;

  /// Pointer to the read-only file data
  const char *read_ptr;

  /// @brief Pointer to writeable file-data, this pointer is only set if `write` is set to true when
  ///          opening the file.
  char *write_ptr;

  /// Length of the open file in bytes.
  s32 size;
};


b32
open_file(const char *filename, File *result, b32 write = false, s32 trunc_to = -1);


b32
close_file(File *file, s32 trunc_to = -1);


#endif
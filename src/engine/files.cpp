#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "types.h"
#include "print.h"
#include "files.h"


b32
open_file(const char *filename, File *result, b32 write, s32 trunc_to)
{
  b32 success = true;

  s32 open_flags = O_RDONLY;
  s32 mmap_protection = PROT_READ;
  s32 mmap_flags = MAP_PRIVATE;
  if (write)
  {
    open_flags = O_RDWR | O_TRUNC;
    mmap_protection = PROT_READ | PROT_WRITE;
    mmap_flags = MAP_SHARED;
  }

  result->fd = open(filename, open_flags);
  if (result->fd == -1)
  {
    print("Failed to open file: \"%s\"\n", filename);
    success = false;
  }
  else
  {
    if (trunc_to < 0)
    {
      // If not re-sizing the file, get the actual size of the file.

      struct stat sb;
      if (fstat(result->fd, &sb) == -1)
      {
        print("Failed to fstat : \"%s\"\n", filename);
        success = false;
      }
      else
      {
        result->size = sb.st_size;
      }
    }
    else
    {
      // Re-size the file

      result->size = trunc_to;
      if (ftruncate(result->fd, result->size) == -1)
      {
        print("Failed to ftruncate file: \"%s\"\n", filename);
        success = false;
      }
    }

    if (success)
    {
      const char *file_ptr = (const char *)mmap(NULL, result->size, mmap_protection, mmap_flags, result->fd, 0);
      if (file_ptr == MAP_FAILED)
      {
        print("Failed to map file: \"%s\"\n", filename);
        success = false;
      }
      else
      {
        result->read_ptr = file_ptr;

        if (write)
        {
          result->write_ptr = (u8 *)file_ptr;
        }
        else
        {
          result->write_ptr = NULL;
        }
      }
    }
  }

  if (!success)
  {
    close(result->fd);
    result->fd = -1;
    result->read_ptr = 0;
    result->write_ptr = 0;
    result->size = 0;
  }

  return success;
}


b32
close_file(File *file, s32 trunc_to)
{
  b32 error = false;

  if (munmap((void *)file->read_ptr, file->size) != 0)
  {
    print("Error unmapping file.\n");
    error = true;
  }

  if (trunc_to >= 0)
  {
    file->size = trunc_to;
    if (ftruncate(file->fd, file->size) == -1)
    {
      print("Failed to truncate file for saving.\n");
    }
  }

  if (close(file->fd) != 0)
  {
    print("Error while closing file descriptor.\n");
    error = true;
  }

  return error;
}
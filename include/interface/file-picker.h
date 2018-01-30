#ifndef FILE_PICKER_H_DEF
#define FILE_PICKER_H_DEF

#include "engine/types.h"
#include "engine/allocate.h"
#include "engine/my-array.h"

#define _TINYDIR_MALLOC(_size) allocate_size((_size), 1)
#define _TINYDIR_FREE(_ptr) un_allocate((_ptr))
#include "header-libs/tinydir.h"

#define FILE_NAME_LIMIT _TINYDIR_FILENAME_MAX

/// @file
///


/// Maintains state for the file picker GUI element
///
struct FilePicker
{
  /// The directory to open the file picker in
  Array::Array<char> root_directory;

  /// The current path which the picker is in, relative to the root directory
  Array::Array<char> current_path;

  /// The currently selected item in the current folder
  s32 current_item;

  /// The currently selected file by the used, only updated as .active switches to false.
  Array::Array<char> selected_file;
};


b32
file_picker(const char *picker_name, FilePicker *picker);


#endif
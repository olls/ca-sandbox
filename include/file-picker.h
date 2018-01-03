#ifndef FILE_PICKER_H_DEF
#define FILE_PICKER_H_DEF

#include "types.h"
#include "allocate.h"

#define _TINYDIR_MALLOC(_size) allocate_size((_size), 1)
#define _TINYDIR_FREE(_ptr) un_allocate((_ptr))
#include "tinydir.h"


/// Maintains state for the file picker GUI element
///
struct FilePicker
{
  /// Is the picker currently open?  Switches to false when the user selects a file.
  b32 active;

  /// The directory to open the file picker in
  const char *root_directory;

  /// The current path which the picker is in, relative to the root directory
  char current_path[_TINYDIR_FILENAME_MAX];

  /// The currently selected item in the current folder
  s32 current_item;

  /// The currently selected file by the used, only updated as .active switches to false.
  char selected_file[_TINYDIR_FILENAME_MAX];
};


void
file_picker(const char *picker_name, FilePicker *picker);


#endif
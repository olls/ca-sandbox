#ifndef FILES_LOADED_STATE_H_DEF
#define FILES_LOADED_STATE_H_DEF

#include "engine/types.h"
#include "engine/my-array.h"

/// @file
///
/// - file states
///   - cells_file_loaded
///   - rule_file_loaded
///
///   - actions:
///     - load rule file
///     - load cells file
///     - unload rule file
///     - unload cells file
///     - new rule file
///     - new cells file
///
///     - if rule file not loaded, cannot not load cells file
///     - if cells file loaded, cannot unload rule file
///


struct FilesLoadedState
{
  b32 rule_file_loaded;
  b32 cells_file_loaded;

  b32 load_cells_file;
  b32 load_rule_file;

  b32 unload_cells_file;
  b32 unload_rule_file;

  b32 error;
  Array::Array<char> errors;
};


void
flag_load_rule_file(FilesLoadedState *files_loaded_state);


void
flag_load_cells_file(FilesLoadedState *files_loaded_state);


void
flag_unload_rule_file(FilesLoadedState *files_loaded_state);


void
flag_unload_cells_file(FilesLoadedState *files_loaded_state);


#endif
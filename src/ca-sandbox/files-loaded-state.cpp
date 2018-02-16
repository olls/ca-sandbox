#include "ca-sandbox/files-loaded-state.h"

#include "engine/my-array.h"
#include "engine/text.h"


void
flag_load_rule_file(FilesLoadedState *files_loaded_state)
{
  files_loaded_state->load_rule_file = true;
}


void
flag_load_cells_file(FilesLoadedState *files_loaded_state)
{
  if (!files_loaded_state->rule_file_loaded)
  {
    files_loaded_state->error |= true;
    append_string(files_loaded_state->errors, new_string("Error: Cannot load cells file without having a rule file loaded first.\n"));
  }
  else
  {
    files_loaded_state->load_cells_file = true;
  }
}


void
flag_unload_rule_file(FilesLoadedState *files_loaded_state)
{
  if (files_loaded_state->cells_file_loaded)
  {
    files_loaded_state->error |= true;
    append_string(files_loaded_state->errors, new_string("Error: Cannot unload rule file, whilst cells file is currently loaded.\n"));
  }
  else
  {
    files_loaded_state->unload_rule_file = true;
  }
}


void
flag_unload_cells_file(FilesLoadedState *files_loaded_state)
{
  files_loaded_state->unload_cells_file = true;
}
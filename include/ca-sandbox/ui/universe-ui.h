#ifndef UNIVERSE_UI_H_DEF
#define UNIVERSE_UI_H_DEF

#include "ca-sandbox/cell-blocks.h"
#include "ca-sandbox/simulate.h"
#include "ca-sandbox/files-loaded-state.h"

#include "ca-sandbox/ui/new-universe-ui.h"

#include "interface/file-picker.h"

#include "engine/types.h"

/// @file
///


struct UniverseUI
{
  b32 file_loaded;

  FilePicker cells_file_picker;
  char loaded_file_name[FILE_NAME_LIMIT];
  b32 save_cells_file;

  NewUniverseUI new_universe_ui;

  u32 edited_cell_block_dim;

  b32 loading_error;
  Array::Array<char> loading_error_message;
};


void
do_universe_ui(UniverseUI *universe_ui, Universe **universe, SimulateOptions *simulate_options, CellInitialisationOptions *cell_initialisation_options, NamedStates *named_states, FilesLoadedState *files_laoded_state);


void
universe_ui_modals(UniverseUI *universe_ui);


#endif
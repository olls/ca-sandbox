#ifndef UNIVERSE_UI_H_DEF
#define UNIVERSE_UI_H_DEF

#include "engine/types.h"

#include "ca-sandbox/universe.h"
#include "ca-sandbox/simulate.h"

#include "interface/file-picker.h"

/// @file
///


struct UniverseUI
{
  FilePicker cells_file_picker;
  b32 reload_cells_file;
  b32 save_cells_file;
};


void
do_universe_ui(UniverseUI *universe_ui, Universe *universe, SimulateOptions *simulate_options, CellInitialisationOptions *cell_initialisation_options, NamedStates *named_states);


#endif
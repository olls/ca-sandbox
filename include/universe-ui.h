#ifndef UNIVERSE_UI_H_DEF
#define UNIVERSE_UI_H_DEF

#include "types.h"

#include "universe.h"
#include "simulate.h"
#include "file-picker.h"


struct UniverseUI
{
  FilePicker cells_file_picker;
};


void
do_universe_ui(UniverseUI *universe_ui, Universe *universe, SimulateOptions *simulate_options, CellInitialisationOptions *cell_initialisation_options, NamedStates *named_states);


#endif
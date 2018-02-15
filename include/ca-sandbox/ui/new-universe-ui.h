#ifndef NEW_UNIVERSE_UI_H_DEF
#define NEW_UNIVERSE_UI_H_DEF

#include "interface/file-picker.h"


struct NewUniverseUI
{
  FilePicker directory_picker;
  char file_name_buffer[FILE_NAME_LIMIT];

  u32 cell_block_dim;

  b32 create_new_universe;
};


void
open_new_universe_ui(NewUniverseUI *new_universe_ui);


void
new_universe_ui(NewUniverseUI *new_universe_ui);


#endif
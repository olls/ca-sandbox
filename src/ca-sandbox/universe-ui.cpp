#include "ca-sandbox/universe-ui.h"

#include "engine/files.h"
#include "engine/text.h"
#include "engine/print.h"

#include "ca-sandbox/load-universe.h"
#include "ca-sandbox/named-states.h"
#include "ca-sandbox/universe.h"
#include "ca-sandbox/simulate.h"
#include "ca-sandbox/cell.h"

#include "imgui/imgui.h"

/// @file
/// @brief  GUI for managing the Universe object
///


/// The GUI elements for managing the .cells file, and the objects which its data fills.
///
void
do_universe_ui(UniverseUI *universe_ui, Universe *universe, SimulateOptions *simulate_options, CellInitialisationOptions *cell_initialisation_options, NamedStates *named_states)
{
  if (ImGui::Begin("Universe"))
  {
    u32 string_length = strlen(universe_ui->cells_file_picker.selected_file);
    ImGui::Text("Cells file: %.*s", string_length, universe_ui->cells_file_picker.selected_file);

    const char *cells_file_picker_name = "Cells file picker";
    if (ImGui::Button("Change cells file"))
    {
      ImGui::OpenPopup(cells_file_picker_name);
      universe_ui->cells_file_picker.current_item = 0;
      universe_ui->cells_file_picker.root_directory = ".";
      copy_string(universe_ui->cells_file_picker.current_path, "cells", 6);
    }

    file_picker(cells_file_picker_name, &universe_ui->cells_file_picker);

    ImGui::SameLine();
    if (ImGui::Button("Load cells file"))
    {
      universe_ui->reload_cells_file = true;
    }

    if (ImGui::Button("Save cells file"))
    {
      universe_ui->save_cells_file = true;
    }
  }

  ImGui::End();
}
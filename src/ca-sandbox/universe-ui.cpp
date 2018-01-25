#include "ca-sandbox/universe-ui.h"

#include "engine/files.h"
#include "engine/text.h"
#include "engine/print.h"

#include "ca-sandbox/load-universe.h"
#include "ca-sandbox/named-states.h"
#include "ca-sandbox/cell-blocks.h"
#include "ca-sandbox/simulate.h"
#include "ca-sandbox/cell.h"

#include "imgui/imgui.h"

/// @file
/// @brief  GUI for managing the Universe object
///


/// The GUI elements for managing the .cells file, and the objects which its data fills.
///
void
do_universe_ui(UniverseUI *universe_ui, Universe **universe_ptr, SimulateOptions *simulate_options, CellInitialisationOptions *cell_initialisation_options, NamedStates *named_states)
{
  if (ImGui::Begin("Universe"))
  {
    if (*universe_ptr != 0)
    {
      u32 string_length = strlen(universe_ui->loaded_file_name);
      ImGui::Text("Cells file: %.*s", string_length, universe_ui->loaded_file_name);
    }

    const char *cells_file_picker_name = "Cells file picker";
    if (ImGui::Button("Change cells file"))
    {
      ImGui::OpenPopup(cells_file_picker_name);
      universe_ui->cells_file_picker.current_item = 0;
      universe_ui->cells_file_picker.root_directory = ".";
      copy_string(universe_ui->cells_file_picker.current_path, "cells", 6);
    }

    if (file_picker(cells_file_picker_name, &universe_ui->cells_file_picker))
    {
      universe_ui->reload_cells_file = true;
    }

    if (*universe_ptr != 0)
    {
      if (ImGui::Button("Save cells file"))
      {
        universe_ui->save_cells_file = true;
      }
    }
  }

  ImGui::End();

  const char *loading_error_modal_title = "Error whist loading new cells file:";
  if (universe_ui->loading_error)
  {
    ImGui::OpenPopup(loading_error_modal_title);
    universe_ui->loading_error = false;
  }

  if (ImGui::BeginPopupModal(loading_error_modal_title))
  {
    ImGui::Text("%.*s", universe_ui->loading_error_message.n_elements, universe_ui->loading_error_message.elements);

    if (ImGui::Button("OK"))
    {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}
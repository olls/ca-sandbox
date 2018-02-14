#include "ca-sandbox/ui/universe-ui.h"

#include "engine/files.h"
#include "engine/text.h"
#include "engine/print.h"

#include "ca-sandbox/load-universe.h"
#include "ca-sandbox/named-states.h"
#include "ca-sandbox/cell-blocks.h"
#include "ca-sandbox/simulate.h"
#include "ca-sandbox/cell.h"
#include "ca-sandbox/re-blockify-cell-blocks.h"

#include "imgui/imgui.h"

/// @file
/// @brief  GUI for managing the Universe object
///


/// The GUI elements for managing the .cells file, and the objects which its data fills.
///
void
do_universe_ui(UniverseUI *universe_ui, Universe **universe_ptr, SimulateOptions *simulate_options, CellInitialisationOptions *cell_initialisation_options, NamedStates *named_states)
{
  if (*universe_ptr != 0)
  {
    ImGui::InputText("Cells filename", universe_ui->loaded_file_name, FILE_NAME_LIMIT);
  }

  const char *cells_file_picker_name = "Cells file picker";
  if (ImGui::Button("Change cells file"))
  {
    ImGui::OpenPopup(cells_file_picker_name);
    universe_ui->cells_file_picker.current_item = 0;

    Array::clear(universe_ui->cells_file_picker.root_directory);
    append_string(universe_ui->cells_file_picker.root_directory, new_string("."));

    Array::clear(universe_ui->cells_file_picker.current_path);
    append_string(universe_ui->cells_file_picker.current_path, new_string("cells"));
  }

  if (file_picker(cells_file_picker_name, &universe_ui->cells_file_picker))
  {
    universe_ui->reload_cells_file = true;
  }

  const char *new_universe_ui_window_name = "New Universe";
  if (ImGui::Button("New Cells File"))
  {
    open_new_universe_ui(&universe_ui->new_universe_ui);
  }
  new_universe_ui(&universe_ui->new_universe_ui);

  if (*universe_ptr != 0)
  {
    if (ImGui::Button("Save cells file"))
    {
      universe_ui->save_cells_file = true;
    }

    ImGui::Text("Cell Block Dimensions");
    ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth()*0.5);
    ImGui::DragInt("###Cell Block Dimensions", (s32 *)&universe_ui->edited_cell_block_dim, 1.0f, 1, 1024, "%0.0f");
    ImGui::SameLine();
    if (ImGui::Button("Reblockify cells file"))
    {
      Universe *old_universe = *universe_ptr;
      Universe *new_universe = allocate(Universe, 1);
      init_cell_hashmap(new_universe);
      new_universe->cell_block_dim = universe_ui->edited_cell_block_dim;

      re_blockify_cell_blocks(old_universe, new_universe);

      destroy_cell_hashmap(old_universe);
      un_allocate(old_universe);

      *universe_ptr = new_universe;
    }
  }
}


void
universe_ui_modals(UniverseUI *universe_ui)
{
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
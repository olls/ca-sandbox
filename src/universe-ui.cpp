#include "universe-ui.h"

#include "files.h"
#include "text.h"
#include "print.h"

#include "load-universe.h"
#include "named-states.h"
#include "universe.h"
#include "simulate.h"
#include "cell.h"

#include "imgui.h"


/// The GUI elements for managing the .cells file, and the objects which its data fills.
///
void
do_universe_ui(UniverseUI *universe_ui, Universe *universe, SimulateOptions *simulate_options, CellInitialisationOptions *cell_initialisation_options, NamedStates *named_states)
{
  if (ImGui::Begin("Universe"))
  {
    u32 string_length = strlen(universe_ui->cells_file_picker.selected_file);
    ImGui::InputText("cells file name", universe_ui->cells_file_picker.selected_file, string_length);

    if (ImGui::Button("Change cells file"))
    {
      universe_ui->cells_file_picker.active = true;
      universe_ui->cells_file_picker.current_item = 0;
      universe_ui->cells_file_picker.root_directory = ".";
      universe_ui->cells_file_picker.current_path[0] = '.'  ;
      universe_ui->cells_file_picker.current_path[1] = '\0';
    }

    if (universe_ui->cells_file_picker.active)
    {
      file_picker("Cells file picker", &universe_ui->cells_file_picker);
    }
    else
    {
      // universe_ui->cells_file_picker.selected_file;
    }

    if (ImGui::Button("Load cells file"))
    {
      // Reload the cells file.
      b32 success = load_universe(universe_ui->cells_file_picker.selected_file, universe, simulate_options, cell_initialisation_options, named_states);
    }
  }

  ImGui::End();
}
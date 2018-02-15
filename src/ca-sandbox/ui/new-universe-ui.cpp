#include "ca-sandbox/ui/new-universe-ui.h"

#include "engine/my-array.h"
#include "engine/text.h"

#include "imgui/imgui.h"

const char NEW_UNIVERSE_UI_WINDOW_NAME[] = "New Universe";


void
open_new_universe_ui(NewUniverseUI *new_universe_ui)
{
  memset(new_universe_ui->file_name_buffer, '\0', FILE_NAME_LIMIT*sizeof(char));

  new_universe_ui->directory_picker.current_item = 0;

  Array::clear(new_universe_ui->directory_picker.root_directory);
  append_string(new_universe_ui->directory_picker.root_directory, new_string("."));

  Array::clear(new_universe_ui->directory_picker.current_path);
  append_string(new_universe_ui->directory_picker.current_path, new_string("cells"));

  Array::clear(new_universe_ui->directory_picker.selected_file);
  append_string(new_universe_ui->directory_picker.selected_file, new_string("./cells"));

  new_universe_ui->cell_block_dim = 16;

  ImGui::OpenPopup(NEW_UNIVERSE_UI_WINDOW_NAME);
}


void
new_universe_ui(NewUniverseUI *new_universe_ui)
{
  ImGui::SetNextWindowSize({0, 0});
  if (ImGui::BeginPopupModal(NEW_UNIVERSE_UI_WINDOW_NAME))
  {
    ImGui::PushID("new-universe-ui");

    const char *directory_picker_name = "New cells file location";
    if (ImGui::Button("Select location"))
    {
      ImGui::OpenPopup(directory_picker_name);
    }

    file_picker(directory_picker_name, &new_universe_ui->directory_picker, true);

    ImGui::InputText("File name", new_universe_ui->file_name_buffer, FILE_NAME_LIMIT);
    ImGui::Text("New file path: %.*s/%s", new_universe_ui->directory_picker.selected_file.n_elements, new_universe_ui->directory_picker.selected_file.elements, new_universe_ui->file_name_buffer);

    ImGui::DragInt("Cell Block Dimensions", (s32 *)&new_universe_ui->cell_block_dim, 1.0f, 1, 1024, "%0.0f");

    if (ImGui::Button("Create"))
    {
      append_string(new_universe_ui->directory_picker.selected_file, new_string("/"));
      append_string(new_universe_ui->directory_picker.selected_file, new_string(new_universe_ui->file_name_buffer));
      new_universe_ui->create_new_universe = true;
      ImGui::CloseCurrentPopup();
    }

    if (ImGui::Button("Cancel"))
    {
      ImGui::CloseCurrentPopup();
    }

    ImGui::PopID();
    ImGui::EndPopup();
  }
}
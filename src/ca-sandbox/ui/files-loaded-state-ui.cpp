#include "ca-sandbox/ui/files-loaded-state-ui.h"

#include "imgui/imgui.h"


void
do_files_loaded_state_ui(FilesLoadedState *files_loaded_state)
{
  const char *error_popup_name = "Loaded File State Error";
  if (files_loaded_state->error)
  {
    ImGui::OpenPopup(error_popup_name);
    files_loaded_state->error = false;
  }

  ImGui::SetNextWindowSize({250, 0});
  if (ImGui::BeginPopupModal(error_popup_name))
  {
    ImGui::TextWrapped("%.*s", files_loaded_state->errors.n_elements, files_loaded_state->errors.elements);

    if (ImGui::Button("OK"))
    {
      Array::clear(files_loaded_state->errors);
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
}

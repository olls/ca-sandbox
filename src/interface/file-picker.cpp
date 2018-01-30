#include "interface/file-picker.h"

#include "engine/types.h"
#include "engine/print.h"
#include "engine/text.h"
#include "engine/util.h"
#include "engine/assert.h"

#include "imgui/imgui.h"

#include "header-libs/tinydir.h"

/// @file
/// @brief  Provides a very simple file browser/picker using ImGui
///


/// This is used to pass the tinydir_dir reference through the ImGui::ListBox callback, and to keep
///   the tinydir_file object in the scope of the LIstBox caller __not inside the callback__
struct TinydirPackage
{
  tinydir_dir dir;
  tinydir_file file;
};


bool
get_filename_from_tinydir_file(void *void_tinydir_package, int idx, const char **out_text)
{
  b32 success = true;

  TinydirPackage *tinydir_package = (TinydirPackage *)void_tinydir_package;

  if (tinydir_readfile_n(&tinydir_package->dir, &tinydir_package->file, idx) < 0)
  {
    success = false;
  }
  else
  {
    *out_text = tinydir_package->file.name;
  }

  return success;
}


b32
file_picker(const char *picker_name, FilePicker *picker)
{
  b32 file_chosen = false;

  if (ImGui::BeginPopupModal(picker_name))
  {
    TinydirPackage tinydir_package = {};

    // Build string of root_directory/current_path
    Array::Array<char> current_directory;
    Array::add(current_directory, picker->root_directory);
    Array::add(current_directory, '/');
    Array::add(current_directory, picker->current_path);
    Array::add(current_directory, '\0');

    tinydir_open_sorted(&tinydir_package.dir, current_directory.elements);

    Array::free_array(current_directory);

    ImGui::PushItemWidth(350);
    ImGui::ListBox("##file-picker", &picker->current_item, &get_filename_from_tinydir_file, &tinydir_package, tinydir_package.dir.n_files, 8);
    ImGui::PopItemWidth();

    if (ImGui::Button("Select"))
    {
      tinydir_file file;
      tinydir_readfile_n(&tinydir_package.dir, &file, picker->current_item);

      if (file.is_dir)
      {
        // Build string for new current_path = current_path/file.name
        append_string(picker->current_path, new_string("/"));
        append_string(picker->current_path, new_string(file.name));

        picker->current_item = 0;

        print("Changing to directory: %.*s\n", picker->current_path.n_elements, picker->current_path.elements);
      }
      else
      {
        // Build string for absolute file path = root_directory/current_path/file.name\0
        Array::clear(picker->selected_file);

        Array::add(picker->selected_file, picker->root_directory);
        Array::add(picker->selected_file, '/');
        Array::add(picker->selected_file, picker->current_path);
        Array::add(picker->selected_file, '/');
        append_string(picker->selected_file, new_string(file.name));
        Array::add(picker->selected_file, '\0');

        print("Selected file: %.*s\n", picker->selected_file.n_elements, picker->selected_file.elements);

        ImGui::CloseCurrentPopup();
        file_chosen = true;
      }
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
      ImGui::CloseCurrentPopup();
    }

    tinydir_close(&tinydir_package.dir);

    ImGui::EndPopup();
  }

  return file_chosen;
}
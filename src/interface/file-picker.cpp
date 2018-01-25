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
    u32 root_length = strlen(picker->root_directory);
    u32 path_length = strlen(picker->current_path);

    u32 current_dirrctory_length = root_length + 1 + path_length + 1;
    char current_dirrctory_data[current_dirrctory_length];
    WriteString current_dirrctory = {
      .start = current_dirrctory_data,
      .current_position = current_dirrctory_data,
      .end = current_dirrctory_data + current_dirrctory_length
    };

    copy_string(current_dirrctory.current_position, picker->root_directory, root_length);
    current_dirrctory.current_position += root_length;

    current_dirrctory.current_position[0] = '/';
    current_dirrctory.current_position += 1;

    copy_string(current_dirrctory.current_position, picker->current_path, path_length);
    current_dirrctory.current_position += path_length;

    current_dirrctory.current_position[0] = '\0';
    current_dirrctory.current_position += 1;

    assert(current_dirrctory.current_position == current_dirrctory.end);

    tinydir_open_sorted(&tinydir_package.dir, current_dirrctory.start);

    ImGui::PushItemWidth(350);
    ImGui::ListBox("##file-picker", &picker->current_item, &get_filename_from_tinydir_file, &tinydir_package, tinydir_package.dir.n_files, 8);
    ImGui::PopItemWidth();

    if (ImGui::Button("Select"))
    {
      tinydir_file file;
      tinydir_readfile_n(&tinydir_package.dir, &file, picker->current_item);

      if (file.is_dir)
      {
        // Build string for new current_path = current_path/file.name\0

        u32 path_length = strlen(picker->current_path);
        u32 file_length = strlen(file.name);

        u32 folder_path_length = path_length + 1 + file_length + 1;
        char folder_path_data[folder_path_length];
        WriteString folder_path = {
          .start = folder_path_data,
          .current_position = folder_path_data,
          .end = folder_path_data + folder_path_length
        };

        copy_string(folder_path.current_position, picker->current_path, path_length);
        folder_path.current_position += path_length;

        folder_path.current_position[0] = '/';
        folder_path.current_position += 1;

        copy_string(folder_path.current_position, file.name, file_length);
        folder_path.current_position += file_length;

        folder_path.current_position[0] = '\0';
        folder_path.current_position += 1;

        assert(folder_path.current_position == folder_path.end);

        // Switch to the selected directory
        copy_string(picker->current_path, folder_path.start, string_length(folder_path));
        picker->current_item = 0;

        print("Changing to directory: %.*s\n", string_length(folder_path), folder_path.start);
      }
      else
      {
        // Build string for absolute file path = root_directory/current_path/file.name\0

        u32 root_length = strlen(picker->root_directory);
        u32 path_length = strlen(picker->current_path);
        u32 file_length = strlen(file.name);

        u32 selected_file_length = root_length + 1 + path_length + 1 + file_length + 1;
        assert(selected_file_length < array_count(picker->selected_file));

        WriteString selected_file = {
          .start = picker->selected_file,
          .current_position = picker->selected_file,
          .end = picker->selected_file + selected_file_length
        };

        copy_string(selected_file.current_position, picker->root_directory, root_length);
        selected_file.current_position += root_length;

        selected_file.current_position[0] = '/';
        selected_file.current_position += 1;

        copy_string(selected_file.current_position, picker->current_path, path_length);
        selected_file.current_position += path_length;

        selected_file.current_position[0] = '/';
        selected_file.current_position += 1;

        copy_string(selected_file.current_position, file.name, file_length);
        selected_file.current_position += file_length;

        selected_file.current_position[0] = '\0';
        selected_file.current_position += 1;

        assert(selected_file.current_position == selected_file.end);

        print("Selected file: %.*s\n", string_length(selected_file), selected_file.start);

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
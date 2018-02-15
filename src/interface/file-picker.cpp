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


b32
file_picker(const char *picker_name, FilePicker *picker, b32 is_directory_picker)
{
  // TODO: Retain tinydir_dir and tinydir_file in FilePicker struct to save pummelling the os

  b32 file_chosen = false;
  tinydir_file file;

  if (ImGui::BeginPopupModal(picker_name))
  {
    tinydir_dir directory_listing = {};

    // Build string of root_directory/current_path
    Array::Array<char, false, 64> current_directory;
    current_directory += picker->root_directory;
    current_directory += '/';
    current_directory += picker->current_path;
    current_directory += '\0';

    tinydir_open_sorted(&directory_listing, current_directory.elements);

    Array::free_array(current_directory);

    b32 item_double_clicked = false;

    ImGui::PushItemWidth(350);
    ImGui::ListBoxHeader("##file-picker", directory_listing.n_files, 8);

    for (u32 file_n = 0;
         file_n < directory_listing.n_files;
         ++file_n)
    {
      tinydir_file file = {};

      if (tinydir_readfile_n(&directory_listing, &file, file_n) == 0)
      {
        b32 currently_selected = file_n == picker->current_item;

        if (ImGui::Selectable(file.name, currently_selected))
        {
          picker->current_item = file_n;
        }
        if (ImGui::IsItemHovered() &&
            ImGui::IsMouseDoubleClicked(0))
        {
          picker->current_item = file_n;
          item_double_clicked = true;
        }
        if (currently_selected)
        {
          ImGui::SetItemDefaultFocus();
        }
      }
    }

    ImGui::ListBoxFooter();
    ImGui::PopItemWidth();

    if (ImGui::Button("Select") || item_double_clicked)
    {
      tinydir_readfile_n(&directory_listing, &file, picker->current_item);

      if (file.is_dir)
      {
        // Build string for new current_path = current_path/file.name
        picker->current_path += '/';
        append_string(picker->current_path, new_string(file.name));

        picker->current_item = 0;

        print("Changing to directory: %.*s\n", picker->current_path.n_elements, picker->current_path.elements);

        // If this picker is for choosing a directory: double clicking enters the directory,
        //   "Select" button picks the directory.
        if (is_directory_picker && !item_double_clicked)
        {
          file_chosen = true;
        }
      }
      else
      {
        file_chosen = true;
      }
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
      ImGui::CloseCurrentPopup();
    }

    if (file_chosen)
    {
      // Build string for absolute file path = root_directory/current_path/file.name\0
      Array::clear(picker->selected_file);

      picker->selected_file += picker->root_directory;
      picker->selected_file += '/';
      picker->selected_file += picker->current_path;

      if (!is_directory_picker)
      {
        picker->selected_file += '/';
        append_string(picker->selected_file, new_string(file.name));
      }

      print("Selected file: %.*s\n", picker->selected_file.n_elements, picker->selected_file.elements);
      ImGui::CloseCurrentPopup();
    }

    tinydir_close(&directory_listing);

    ImGui::EndPopup();
  }

  return file_chosen;
}

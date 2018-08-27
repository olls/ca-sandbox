#include "ca-sandbox/ui/cell-regions-ui.h"

#include "ca-sandbox/cell-block-coordinate-system.h"
#include "ca-sandbox/cell-regions.h"
#include "ca-sandbox/universe.h"
#include "ca-sandbox/cell-tools.h"

#include "ca-sandbox/ui/cell-selections-ui.h"

#include "engine/vectors.h"
#include "engine/print.h"

#include "imgui/imgui.h"

#include <SDL2/SDL.h>


void
display_regions(CellRegionsUI *cell_regions_ui, CellRegions *cell_regions)
{
  for (u32 region_index = 0;
       region_index < cell_regions->regions.n_elements;
       ++region_index)
  {
    ImGui::PushID(region_index);

    CellRegion& cell_region = cell_regions->regions[region_index];
    ImGui::Text("Name: %.*s", string_length(cell_region.name), cell_region.name.start);

    if (cell_regions_ui->placing_region &&
        cell_regions_ui->placing_region_index == region_index)
    {
      ImGui::Text("Place the region in the universe");
    }
    else if (ImGui::Button("Place region in universe"))
    {
      cell_regions_ui->placing_region = true;
      cell_regions_ui->placing_region_index = region_index;
    }

    if (ImGui::Button("Delete region"))
    {
      delete_region(cell_regions, region_index);
    }
    else
    {
      if (cell_region.texture)
      {
        ImTextureID tex_id = (void *)(intptr_t)cell_region.texture;
        ImGui::Image(tex_id, s32vec2_to_vec2(cell_region.texture_size), ImVec2(0,0), ImVec2(1,1), ImColor(0xFF, 0xFF, 0xFF, 0xFF), ImColor(0xFF, 0xFF, 0xFF, 0x80));
      }
    }

    ImGui::PopID();
  }
}


void
do_cell_regions_ui(CellRegionsUI *cell_regions_ui, CellRegions *cell_regions, CellSelectionsUI *cell_selections_ui)
{
  if (cell_regions->clipboard_region_in_use)
  {
    ImGui::Text("Region in Clipboard");
    if (cell_regions->clipboard_region.texture)
    {
      ImTextureID tex_id = (void *)(intptr_t)cell_regions->clipboard_region.texture;
      ImGui::Image(tex_id, s32vec2_to_vec2(cell_regions->clipboard_region.texture_size), ImVec2(0,0), ImVec2(1,1), ImColor(0xFF, 0xFF, 0xFF, 0xFF), ImColor(0xFF, 0xFF, 0xFF, 0x80));
    }
  }

  if (!cell_selections_ui->selection_made)
  {
    ImGui::TextWrapped("Select a region of cells using shift + mouse drag to create a new region");
  }
  else
  {
    ImGui::InputText("New region name", cell_regions_ui->new_region_name_buffer, BUFFER_NAME_MAX_LENGTH);
    const char *give_region_name_label = "Give region a name first";
    if (ImGui::Button("Make new region from selection"))
    {
      if (strlen(cell_regions_ui->new_region_name_buffer) == 0)
      {
        ImGui::OpenPopup(give_region_name_label);
      }
      else
      {
        cell_selections_ui->selection_made = false;
        cell_regions_ui->make_new_region = true;
      }
    }

    ImGui::SetNextWindowContentSize({200, 0});
    if (ImGui::BeginPopupModal(give_region_name_label))
    {
      if (ImGui::Button("OK"))
      {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
  }

  display_regions(cell_regions_ui, cell_regions);
}


/// Performs updates to the CellRegions object based on the state of the CellRegionsUI
///
/// Creates new named regions, and manages clipboard regions
///
void
update_cell_regions(CellRegionsUI *cell_regions_ui, CellRegions *cell_regions, CellSelectionsUI *cell_selections_ui, Universe *universe,
                    GLuint minimap_framebuffer, CellDrawing *cell_drawing, CellInstancing *cell_instancing, OpenGL_Buffer *cell_vertices_buffer,
                    UniversePosition mouse_universe_position, b32 *mouse_click_consumed)
{
  // Make new named region
  //
  if (cell_regions_ui->make_new_region)
  {
    cell_regions_ui->make_new_region = false;
    Array::new_element(cell_regions->regions) = make_new_region(cell_selections_ui, universe, cell_regions_ui->new_region_name_buffer, minimap_framebuffer, cell_drawing, cell_instancing, cell_vertices_buffer);
  }

  b32 copy_region = ImGui::IsKeyDown(SDL_SCANCODE_LCTRL) &&
                    ImGui::IsKeyPressed(SDL_SCANCODE_C);

  b32 cut_region = ImGui::IsKeyDown(SDL_SCANCODE_LCTRL) &&
                   ImGui::IsKeyPressed(SDL_SCANCODE_X);

  // copy/cut region to clipboard buffer
  //
  if (cell_selections_ui->selection_made &&
      (copy_region || cut_region))
  {
    cell_regions->clipboard_region = make_new_region(cell_selections_ui, universe, cell_regions_ui->new_region_name_buffer, minimap_framebuffer, cell_drawing, cell_instancing, cell_vertices_buffer);
    cell_regions->clipboard_region_in_use = true;
    cell_selections_ui->selection_made = false;
  }

  if (cut_region)
  {
    set_cells_to_state(cell_selections_ui, universe, 0);
  }

  // Start placing clipboard region
  //
  if (cell_regions->clipboard_region_in_use &&
      ImGui::IsKeyDown(SDL_SCANCODE_LCTRL) &&
      ImGui::IsKeyPressed(SDL_SCANCODE_V))
  {
    cell_regions_ui->placing_clipboard_region = true;
  }

  // Place region in universe
  //
  if ((cell_regions_ui->placing_region ||
       cell_regions_ui->placing_clipboard_region) &&
      ImGui::IsMouseClicked(0) &&
      !*mouse_click_consumed)
  {
    CellRegion *placing_region = 0;
    if (cell_regions_ui->placing_region)
    {
      placing_region = Array::get(cell_regions->regions, cell_regions_ui->placing_region_index);
    }
    else
    {
      placing_region = &cell_regions->clipboard_region;
    }
    *mouse_click_consumed = true;
    cell_regions_ui->placing_region = false;
    cell_regions_ui->placing_clipboard_region = false;
    add_region_to_universe(universe, placing_region, mouse_universe_position);
  }
}
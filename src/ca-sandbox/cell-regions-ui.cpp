#include "ca-sandbox/cell-regions-ui.h"

#include "ca-sandbox/cell-block-coordinate-system.h"
#include "ca-sandbox/cell-regions.h"
#include "ca-sandbox/universe.h"
#include "ca-sandbox/cell-selections-ui.h"

#include "engine/vectors.h"
#include "engine/print.h"

#include "imgui/imgui.h"


void
do_cell_regions_ui(CellRegionsUI *cell_regions_ui, CellRegions *cell_regions, Universe *universe, CellSelectionsUI *cell_selections_ui)
{
  if (ImGui::Begin("Cell Regions"))
  {
    if (universe != 0)
    {
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
            cell_regions_ui->make_new_region = true;
          }
        }

        if (ImGui::BeginPopupModal(give_region_name_label))
        {
          if (ImGui::Button("OK"))
          {
            ImGui::CloseCurrentPopup();
          }
          ImGui::EndPopup();
        }
      }
    }

    for (u32 region_index = 0;
         region_index < cell_regions->regions.n_elements;
         ++region_index)
    {
      CellRegion& cell_region = cell_regions->regions[region_index];
      ImGui::Text("Name: %.*s", string_length(cell_region.name), cell_region.name.start);

      if (cell_region.texture)
      {
        ImTextureID tex_id = (void *)(intptr_t)cell_region.texture;
        ImGui::Image(tex_id, s32vec2_to_vec2(cell_region.texture_size), ImVec2(0,0), ImVec2(1,1), ImColor(0xFF, 0xFF, 0xFF, 0xFF), ImColor(0xFF, 0xFF, 0xFF, 0x80));
      }
    }
  }

  ImGui::End();
}

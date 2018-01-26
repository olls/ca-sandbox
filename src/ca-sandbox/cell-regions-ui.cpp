#include "ca-sandbox/cell-regions-ui.h"

#include "ca-sandbox/cell-block-coordinate-system.h"
#include "ca-sandbox/cell-regions.h"
#include "ca-sandbox/universe.h"
#include "ca-sandbox/cell-drawing.h"
#include "ca-sandbox/cell-selections-ui.h"
#include "ca-sandbox/minimap.h"

#include "engine/opengl-buffer.h"
#include "engine/vectors.h"
#include "engine/print.h"

#include "imgui/imgui.h"


void
make_region_texture(CellRegion *region, Universe *universe, GLuint minimap_framebuffer, CellDrawing *cell_drawing, CellInstancing *cell_instancing, OpenGL_Buffer *cell_vertices_buffer)
{
  region->texture_size = {200, 150};
  region->texture = create_minimap_texture(region->texture_size, minimap_framebuffer);

  if (region->texture != 0)
  {
    upload_cell_instances(&region->cell_blocks, cell_instancing);
    draw_minimap_texture(&region->cell_blocks, cell_instancing, cell_drawing, cell_vertices_buffer, minimap_framebuffer, region->texture, region->texture_size);
  }
}


// TODO:  Instead of passing all the cell drawing objects through this function, return a struct
//        indicating a cell region to create.  Main loop then calls make_region_texture.
void
do_cell_regions_ui(CellRegionsUI *cell_regions_ui, CellRegions *cell_regions, Universe *universe, CellSelectionsUI *cell_selections_ui, GLuint minimap_framebuffer, CellDrawing *cell_drawing, CellInstancing *cell_instancing, OpenGL_Buffer *cell_vertices_buffer, UniversePosition mouse_universe_pos, b32 *mouse_click_consumed)
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
            CellRegion *new_region = make_new_region(cell_regions, universe, cell_regions_ui->new_region_name_buffer, cell_selections_ui->selection_start, cell_selections_ui->selection_end);
            cell_regions_ui->new_region_name_buffer[0] = '\0';

            make_region_texture(new_region, universe, minimap_framebuffer, cell_drawing, cell_instancing, cell_vertices_buffer);
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

#include "ca-sandbox/selection-tools-ui.h"

#include "ca-sandbox/cell-selections-ui.h"
#include "ca-sandbox/universe.h"

#include "engine/print.h"

#include "imgui/imgui.h"


void
set_null(CellSelectionsUI *cell_selections_ui, Universe *universe, CellInitialisationOptions *cell_initialisation_options)
{
  s32vec2 start_block = cell_selections_ui->selection_start.cell_block_position;
  s32vec2 end_block = cell_selections_ui->selection_end.cell_block_position;

  s32vec2 start_cell = vec2_to_s32vec2(vec2_multiply(cell_selections_ui->selection_start.cell_position, universe->cell_block_dim));
  s32vec2 end_cell = vec2_to_s32vec2(vec2_multiply(cell_selections_ui->selection_end.cell_position, universe->cell_block_dim));

  for (u32 cell_block_slot = 0;
       cell_block_slot < universe->hashmap_size;
       ++cell_block_slot)
  {
    CellBlock *cell_block = universe->hashmap[cell_block_slot];

    while (cell_block != 0)
    {
      if (cell_block->block_position.x >= start_block.x &&
          cell_block->block_position.y >= start_block.y &&
          cell_block->block_position.x <= end_block.x &&
          cell_block->block_position.y <= end_block.y)
      {
        s32vec2 this_block_start_cell = {-1, -1};
        s32vec2 this_block_end_cell = {-1, -1};

        if (cell_block->block_position.x == start_block.x)
        {
          this_block_start_cell.x = start_cell.x;
        }
        if (cell_block->block_position.y == start_block.y)
        {
          this_block_start_cell.y = start_cell.y;
        }
        if (cell_block->block_position.x == end_block.x)
        {
          this_block_end_cell.x = end_cell.x;
        }
        if (cell_block->block_position.y == end_block.y)
        {
          this_block_end_cell.y = end_cell.y;
        }

        init_cells(universe, cell_initialisation_options, cell_block, cell_block->block_position, this_block_start_cell, this_block_end_cell);
      }

      cell_block = cell_block->next_block;
    }
  }
}


void
do_selection_tools_ui(CellSelectionsUI *cell_selections_ui, Universe *universe, CellInitialisationOptions *cell_initialisation_options)
{
  if (ImGui::Begin("Selection Tools"))
  {
    if (cell_selections_ui->selection_made)
    {

      ImGui::Value("start_block", cell_selections_ui->selection_start.cell_block_position);
      ImGui::Value("end_block", cell_selections_ui->selection_end.cell_block_position);

      ImGui::Value("start_cell", cell_selections_ui->selection_start.cell_position);
      ImGui::Value("end_cell", cell_selections_ui->selection_end.cell_position);

      if (ImGui::Button("Set selection to null"))
      {
        set_null(cell_selections_ui, universe, cell_initialisation_options);
      }
    }
  }

  ImGui::End();
}
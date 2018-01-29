#include "ca-sandbox/cell-tools-ui.h"

#include "ca-sandbox/cell-tools.h"

#include "imgui/imgui.h"


void
do_cell_tools_ui(CellTools *cell_tools)
{
  if (ImGui::Button("Set selection to null"))
  {
    cell_tools->flags |= CellToolFlags__SetSelectionNull;
  }

  if (ImGui::Button("Delete null cell blocks"))
  {
    cell_tools->flags |= CellToolFlags__DeleteNullBlocks;
  }
}
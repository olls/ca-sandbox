#include "ca-sandbox/ui/simulate-options-ui.h"

#include "ca-sandbox/simulate.h"

#include "imgui/imgui.h"

/// @file
/// @brief  GUI elements for managing the SimulationOptions struct
///


/// Provides GUI elements for modifying the SimulateOptions object
void
do_simulate_options_ui(SimulateOptions *simulate_options, Universe *universe)
{
  if (universe != 0)
  {
    ImGui::Combo("Border type", (s32*)(&simulate_options->border.type), "Fixed\0Infinite\0Torus\0\0");

    if (simulate_options->border.type != BorderType::INFINITE)
    {
      ImGui::DragInt2("Min corner block", &simulate_options->border.min_corner_block.es[0]);
      ImGui::DragInt2("Min corner cell", &simulate_options->border.min_corner_cell.es[0], 1, 0, universe->cell_block_dim);
      ImGui::DragInt2("Max corner block", &simulate_options->border.max_corner_block.es[0]);
      ImGui::DragInt2("Max corner cell", &simulate_options->border.max_corner_cell.es[0], 1, 0, universe->cell_block_dim);
    }
  }
}
#include "simulation-ui.h"

#include "imgui.h"

#include "simulate.h"


/// Provides GUI elements for modifying the SimulateOptions object
///
void
simulate_ui(SimulateOptions *simulate_options, Universe *universe)
{
  if (ImGui::Begin("Simulate Options"))
  {

    ImGui::DragInt2("Min corner block", &simulate_options->border.min_corner_block.es[0]);
    ImGui::DragInt2("Min corner cell", &simulate_options->border.min_corner_cell.es[0], 1, 0, universe->cell_block_dim);
    ImGui::DragInt2("Max corner block", &simulate_options->border.max_corner_block.es[0]);
    ImGui::DragInt2("Max corner cell", &simulate_options->border.max_corner_cell.es[0], 1, 0, universe->cell_block_dim);
  }

  ImGui::End();
}
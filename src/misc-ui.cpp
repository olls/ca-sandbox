#include "misc-ui.h"

#include "types.h"
#include "print.h"
#include "imgui.h"

/// @file
/// @brief  Provides GUI for misc items which don't fall into the other windows
///


void
miscellaneous_ui(MiscUI *misc_ui, u32 last_simulation_delta)
{
  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoCollapse;

  ImGui::SetNextWindowSize(ImVec2(256, 256), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Cellular Automaton Sandbox", NULL, window_flags))
  {
    ImGui::PushItemWidth(100);
    ImGui::DragFloat("Simulations per second", &misc_ui->sim_frequency, 1, 0, 10000);
    ImGui::PopItemWidth();

    if (ImGui::Button("Pause Simulation"))
    {
      misc_ui->simulating = false;
    }
    ImGui::SameLine();

    if (ImGui::Button("Run Simulation"))
    {
      misc_ui->simulating = true;
    }

    misc_ui->step_simulation = ImGui::Button("Step Simulation");

    ImGui::Text("Simulation Time: %.3fms", last_simulation_delta * 0.001);
  }

  ImGui::End();
}
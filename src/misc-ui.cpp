#include "misc-ui.h"

#include "types.h"
#include "print.h"
#include "imgui.h"


void
miscellaneous_ui(MiscUI *misc_ui)
{
  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoCollapse;

  ImGui::SetNextWindowSize(ImVec2(256, 256), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Cellular Automaton Sandbox", NULL, window_flags))
  {
    ImGui::DragFloat("Simulation frames per second", &misc_ui->sim_frequency, 1, 0, 10000);

    if (ImGui::Button("Pause Simulation"))
    {
      misc_ui->sim_frequency = 0;
    }
    ImGui::SameLine();

    misc_ui->step_simulation = ImGui::Button("Step Simulation");
  }

  ImGui::End();
}
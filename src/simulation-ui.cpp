#include "simulation-ui.h"

#include "types.h"
#include "print.h"
#include "imgui.h"

/// @file
/// @brief  Provides GUI for controlling the simulation
///
/// simulator mode
/// editor mode
///
/// run simulator -> simulator mode
/// edit mode -> editor mode
///


void
do_simulation_ui(SimulationUI *simulation, u32 last_simulation_delta, b32 *reload_universe)
{
  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoCollapse;

  ImGui::SetNextWindowSize(ImVec2(256, 256), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Simulation UI", NULL, window_flags))
  {

    ImGui::Text("Mode: %s", simulation->mode == Mode::SIMULATOR ? "Simulator" : "Editor");

    if (simulation->mode == Mode::EDITOR)
    {
      if (ImGui::Button("Run simulation"))
      {
        simulation->simulating = false;
        simulation->mode = Mode::SIMULATOR;
      }

    }
    else if (simulation->mode == Mode::SIMULATOR)
    {
      ImGui::PushItemWidth(100);
      ImGui::DragFloat("Simulations per second", &simulation->sim_frequency, 1, 0, 10000);
      ImGui::PopItemWidth();

      if (simulation->simulating)
      {
        if (ImGui::Button("Pause Simulation"))
        {
          simulation->simulating = false;
        }
      }
      else
      {
        if (ImGui::Button("Un-pause Simulation"))
        {
          simulation->simulating = true;
        }

        ImGui::SameLine();
        simulation->step_simulation = ImGui::Button("Step Simulation");
      }

      ImGui::Text("Simulation Time: %.3fms", last_simulation_delta * 0.001);

      if (ImGui::Button("End simulation"))
      {
        simulation->simulating = false;
        simulation->mode = Mode::EDITOR;
        *reload_universe = true;
      }
    }
  }

  ImGui::End();
}
#include "simulation-ui.h"

#include "types.h"
#include "print.h"
#include "timing.h"

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
do_simulation_ui(SimulationUI *simulation, u64 frame_start, b32 *reload_universe)
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
        simulation->simulation_step = 0;
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
          simulation->last_sim_time = frame_start;
        }

        ImGui::SameLine();
        simulation->step_simulation = ImGui::Button("Step Simulation");
      }

      ImGui::Text("Simulation Time: %.3fms", simulation->last_simulation_delta * 0.001);
      ImGui::Text("Simulation Step: %lu", simulation->simulation_step);

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
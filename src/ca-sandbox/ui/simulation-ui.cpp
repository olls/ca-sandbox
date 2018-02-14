#include "ca-sandbox/ui/simulation-ui.h"

#include "engine/types.h"
#include "engine/print.h"
#include "engine/timing.h"
#include "engine/human-time.h"

#include "imgui/imgui.h"

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
do_simulation_ui(SimulationUI *simulation_ui, u64 frame_start, b32 rule_tree_built, b32 *reload_universe, b32 *save_universe)
{
  ImGui::Text("Mode: %s", simulation_ui->mode == Mode::SIMULATOR ? "Simulator" : "Editor");

  if (simulation_ui->mode == Mode::EDITOR)
  {
    if (ImGui::Button("Save Universe and Run Simulation"))
    {
      *save_universe = true;
      simulation_ui->simulating = false;
      simulation_ui->mode = Mode::SIMULATOR;
      simulation_ui->simulation_step = 0;
    }
  }
  else if (simulation_ui->mode == Mode::SIMULATOR)
  {
    if (!rule_tree_built)
    {
      ImGui::Text("Rule tree not built");
      simulation_ui->simulating = false;
    }
    else
    {
      if (simulation_ui->simulating)
      {
        if (ImGui::Button("Pause Simulation"))
        {
          simulation_ui->simulating = false;
        }
      }
      else
      {
        if (ImGui::Button("Un-pause Simulation"))
        {
          simulation_ui->simulating = true;
          simulation_ui->last_sim_time = frame_start;
        }

        ImGui::SameLine();
        simulation_ui->step_simulation = ImGui::Button("Step Simulation");
      }

      ImGui::PushItemWidth(100);
      ImGui::DragFloat("Simulations per second", &simulation_ui->sim_frequency, 1, 0, 10000);
      ImGui::PopItemWidth();

      const char *unit;

      r32 human_last_simulation_delta = human_time(simulation_ui->last_simulation_delta, &unit);
      ImGui::Text("Simulation Time: %.2f %s", human_last_simulation_delta, unit);

      r32 human_last_simulation_delta_per_cell_block = human_time(simulation_ui->last_simulation_delta_per_cell_block, &unit);
      ImGui::Text("Simulation Time per Cell Block: %.3f %s", human_last_simulation_delta_per_cell_block, unit);

      ImGui::Text("Simulation Step: %lu", simulation_ui->simulation_step);

      r32 human_cumulative_sim_time = human_time(simulation_ui->simulation_delta_cumulative_average, &unit);
      ImGui::Text("Cumulative Moving Average Simulation Time: %.2f %s", human_cumulative_sim_time, unit);
    }

    if (ImGui::Button("End simulation"))
    {
      simulation_ui->simulating = false;
      simulation_ui->mode = Mode::EDITOR;
      *reload_universe = true;
    }
  }
}
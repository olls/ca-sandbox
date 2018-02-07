#include "ca-sandbox/main-gui.h"

#include "ca-sandbox/ca-sandbox.h"
#include "ca-sandbox/rule-ui.h"
#include "ca-sandbox/simulation-ui.h"
#include "ca-sandbox/simulate-options-ui.h"
#include "ca-sandbox/universe-ui.h"
#include "ca-sandbox/named-states-ui.h"
#include "ca-sandbox/cell-regions-ui.h"
#include "ca-sandbox/cell-tools-ui.h"

#include "imgui/imgui.h"
#include "imgui/imgui_tabs.h"


void
setup_imgui_style()
{
  ImGuiStyle& styles = ImGui::GetStyle();
  ImGui::StyleColorsClassic(&styles);

  // Non transparent windows
  styles.Colors[ImGuiCol_WindowBg].w = 1;
  styles.Colors[ImGuiCol_PopupBg]                = ImVec4(0.11f, 0.11f, 0.14f, 1.00f);
  styles.Colors[ImGuiCol_TitleBg]                = ImVec4(0.27f, 0.27f, 0.54f, 1.00f);
  styles.Colors[ImGuiCol_TitleBgActive]          = ImVec4(0.32f, 0.32f, 0.63f, 1.00f);
  styles.Colors[ImGuiCol_ModalWindowDarkening]   = ImVec4(0.20f, 0.20f, 0.20f, 0.43f);

  styles.WindowBorderSize = 1;
  styles.WindowTitleAlign = {0.5, 0.5};

  styles.WindowRounding = 0;
  styles.PopupRounding = 3;
  styles.ChildRounding = 0;
  styles.FrameRounding = 1;
  styles.ScrollbarRounding = 2;
  styles.GrabRounding = 1;
  styles.DisplaySafeAreaPadding = {-1, -1};
}


void
do_main_gui(CA_SandboxState *state, s32vec2 window_size_s32, UniversePosition mouse_universe_pos, b32 *mouse_click_consumed)
{
  const ImGuiWindowFlags imgui_window_flags = ImGuiWindowFlags_NoResize |
                                              ImGuiWindowFlags_NoMove |
                                              ImGuiWindowFlags_NoCollapse |
                                              ImGuiWindowFlags_HorizontalScrollbar;

  vec2 window_size = s32vec2_to_vec2(window_size_s32);

  vec2 window_bottom_right = vec2_multiply(window_size, {1, 1});
  vec2 window_one_past_bottom_right = vec2_add(window_bottom_right, {1, 1});
  vec2 window_top_right = vec2_multiply(window_size, {1, 0});
  vec2 window_one_past_top_right = vec2_add(window_top_right, {1, -1});

  vec2 window_bottom_left = vec2_multiply(window_size, {0, 1});
  vec2 window_one_past_bottom_left = vec2_add(window_bottom_left, {-1, 1});
  vec2 window_top_left = vec2_multiply(window_size, {0, 0});
  vec2 window_one_past_top_left = vec2_add(window_top_left, {-1, -1});

  r32 half_window_height_plus_one = (window_size.y * 0.5) + 1;
  r32 third_window_height_plus_one = (window_size.y * (1.0/3.0)) + 1;

  r32 left_side_bar_width = 0;
  if (state->left_side_bar_open)
  {
    left_side_bar_width = window_size.x * 0.25;
  }
  r32 right_side_bar_width = 0;
  if (state->right_side_bar_open)
  {
    right_side_bar_width = window_size.x * 0.25;
  }

  if (state->right_side_bar_open)
  {
    ImGui::SetNextWindowPos(window_one_past_bottom_right, ImGuiCond_Always, {1, 1});
    ImGui::SetNextWindowSize({right_side_bar_width, half_window_height_plus_one});

    if (ImGui::Begin("Universe", NULL, imgui_window_flags))
    {
      ImGui::BeginTabBar("universe", ImGuiTabBarFlags_NoReorder);

      if (ImGui::TabItem("Cells File"))
      {
        do_universe_ui(&state->universe_ui, &state->universe, &state->simulate_options, &state->cell_initialisation_options, &state->loaded_rule.config.named_states);
      }

      if (ImGui::TabItem("Simulation"))
      {
        do_simulation_ui(&state->simulation_ui, state->frame_timing.frame_start, state->loaded_rule.rule_tree_built, &state->universe_ui.reload_cells_file, &state->universe_ui.save_cells_file);
      }

      if (ImGui::TabItem("Simulation Options"))
      {
        do_simulate_options_ui(&state->simulate_options, state->universe);
      }

      ImGui::EndTabBar();

      // ImGui::SetTabItemClosed(const char* label); // Optional, allows to bypass a frame of flicker in situation where tab is closed programmatically
      // ImGui::SetTabItemSelected(const char* label);
    }
    ImGui::End();

    ImGui::SetNextWindowPos(window_one_past_top_right, ImGuiCond_Always, {1, 0});
    ImGui::SetNextWindowSize({right_side_bar_width, half_window_height_plus_one});

    if (ImGui::Begin("Universe Editing", NULL, imgui_window_flags))
    {
      ImGui::BeginTabBar("universe editing", ImGuiTabBarFlags_NoReorder);

      if (ImGui::TabItem("Cell Regions"))
      {
        do_cell_regions_ui(&state->cell_regions_ui, &state->cell_regions, state->universe, &state->cell_selections_ui, mouse_universe_pos, mouse_click_consumed);
      }

      if (ImGui::TabItem("Cell Tools"))
      {
        do_cell_tools_ui(&state->cell_tools, &state->loaded_rule.config.named_states);
      }

      ImGui::EndTabBar();
    }
    ImGui::End();
  }

  universe_ui_modals(&state->universe_ui);

  if (state->left_side_bar_open)
  {
    ImGui::SetNextWindowPos(window_one_past_top_left, ImGuiCond_Always, {0, 0});
    ImGui::SetNextWindowSize({left_side_bar_width, 2*third_window_height_plus_one});

    if (ImGui::Begin("Rules File Editor", NULL, imgui_window_flags))
    {
      do_rule_ui(&state->rule_ui, &state->loaded_rule, &state->rule_creation_thread);
    }
    ImGui::End();

    ImGui::SetNextWindowPos(window_one_past_bottom_left, ImGuiCond_Always, {0, 1});
    ImGui::SetNextWindowSize({left_side_bar_width, third_window_height_plus_one});

    if (ImGui::Begin("Rule States Editor", NULL, imgui_window_flags))
    {
      do_named_states_ui(&state->loaded_rule.config, &state->cells_editor.active_state);
    }
    ImGui::End();
  }

  // Menu bar for opening and closing side bars.  Created after side bars to remove frame of flicker
  //   on change.
  if (ImGui::BeginCustomMainMenuBar({left_side_bar_width-1, 0}, {right_side_bar_width-1, 0}))
  {
    if (state->left_side_bar_open)
    {
      if (ImGui::MenuItem("< Close"))
      {
        state->left_side_bar_open = false;
      }
    }
    else
    {
      if (ImGui::MenuItem("> Open"))
      {
        state->left_side_bar_open = true;
      }
    }

    ImGui::SetCursorPosX(ImGui::GetWindowSize().x-60);
    if (state->right_side_bar_open)
    {
      if (ImGui::MenuItem("Close >"))
      {
        state->right_side_bar_open = false;
      }
    }
    else
    {
      if (ImGui::MenuItem("Open <"))
      {
        state->right_side_bar_open = true;
      }
    }

    // if (ImGui::BeginMenu("File"))
    // {
    //   ImGui::EndMenu();
    // }
    // if (ImGui::BeginMenu("Edit"))
    // {
    //   if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
    //   if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
    //   ImGui::Separator();
    //   if (ImGui::MenuItem("Cut", "CTRL+X")) {}
    //   if (ImGui::MenuItem("Copy", "CTRL+C")) {}
    //   if (ImGui::MenuItem("Paste", "CTRL+V")) {}
    //   ImGui::EndMenu();
    // }
    ImGui::EndMainMenuBar();
  }
}
#include "ca-sandbox/main-gui.h"

#include "ca-sandbox/ca-sandbox.h"
#include "ca-sandbox/ui/rule-ui.h"
#include "ca-sandbox/ui/simulation-ui.h"
#include "ca-sandbox/ui/simulate-options-ui.h"
#include "ca-sandbox/ui/universe-ui.h"
#include "ca-sandbox/ui/named-states-ui.h"
#include "ca-sandbox/ui/cell-regions-ui.h"
#include "ca-sandbox/ui/cell-tools-ui.h"
#include "ca-sandbox/ui/files-loaded-state-ui.h"

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
do_main_gui(CA_SandboxState *state, s32vec2 screen_size_s32)
{
  const ImGuiWindowFlags imgui_window_flags = ImGuiWindowFlags_NoMove |
                                              ImGuiWindowFlags_NoCollapse |
                                              ImGuiWindowFlags_HorizontalScrollbar |
                                              ImGuiWindowFlags_ResizeFromAnySide;

  vec2 screen_size = s32vec2_to_vec2(screen_size_s32);

  vec2 window_bottom_right = vec2_multiply(screen_size, {1, 1});
  vec2 window_one_past_bottom_right = vec2_add(window_bottom_right, {1, 1});
  vec2 window_top_right = vec2_multiply(screen_size, {1, 0});
  vec2 window_one_past_top_right = vec2_add(window_top_right, {1, -1});

  vec2 window_bottom_left = vec2_multiply(screen_size, {0, 1});
  vec2 window_one_past_bottom_left = vec2_add(window_bottom_left, {-1, 1});
  vec2 window_top_left = vec2_multiply(screen_size, {0, 0});
  vec2 window_one_past_top_left = vec2_add(window_top_left, {-1, -1});

  if (state->right_side_bar_open)
  {
    ImGui::SetNextWindowPos(window_one_past_bottom_right, ImGuiCond_Always, {1, 1});

    // If not set yet, let ImGui set the size
    if (state->right_side_bar_width != 0)
    {
      ImGui::SetNextWindowSize({state->right_side_bar_width, state->right_side_bar_split+1});
    }

    if (ImGui::Begin("Universe", NULL, imgui_window_flags))
    {
      ImGui::BeginTabBar("universe", ImGuiTabBarFlags_NoReorder);

      if (ImGui::TabItem("Cells File"))
      {
        do_universe_ui(&state->universe_ui, &state->universe, &state->simulate_options, &state->cell_initialisation_options, &state->loaded_rule.config.named_states, &state->files_loaded_state);
      }

      if (ImGui::TabItem("Simulation"))
      {
        do_simulation_ui(&state->simulation_ui, state->frame_timing.frame_start, state->loaded_rule.rule_tree_built, &state->files_loaded_state, &state->universe_ui.save_cells_file);
      }

      if (ImGui::TabItem("Simulation Options"))
      {
        do_simulate_options_ui(&state->simulate_options, state->universe);
      }

      ImGui::EndTabBar();

      state->right_side_bar_width = ImGui::GetWindowSize().x;
      state->right_side_bar_split = ImGui::GetWindowSize().y-1;

      // ImGui::SetTabItemClosed(const char* label); // Optional, allows to bypass a frame of flicker in situation where tab is closed programmatically
      // ImGui::SetTabItemSelected(const char* label);
    }
    ImGui::End();

    ImGui::SetNextWindowPos(window_one_past_top_right, ImGuiCond_Always, {1, 0});

    // If not set yet, let ImGui set the size
    if (state->right_side_bar_width != 0)
    {
      ImGui::SetNextWindowSize({state->right_side_bar_width, screen_size.y-state->right_side_bar_split+1});
    }

    if (ImGui::Begin("Universe Editing", NULL, imgui_window_flags))
    {
      ImGui::BeginTabBar("universe editing", ImGuiTabBarFlags_NoReorder);

      if (ImGui::TabItem("Cell Regions"))
      {
        do_cell_regions_ui(&state->cell_regions_ui, &state->cell_regions, &state->cell_selections_ui);
      }

      if (ImGui::TabItem("Cell Tools"))
      {
        do_cell_tools_ui(&state->cell_tools, &state->loaded_rule.config.named_states);
      }

      ImGui::EndTabBar();

      state->right_side_bar_width = ImGui::GetWindowSize().x;
      state->right_side_bar_split = screen_size.y - (ImGui::GetWindowSize().y-1);
    }
    ImGui::End();
  }

  universe_ui_modals(&state->universe_ui);

  if (state->left_side_bar_open)
  {
    ImGui::SetNextWindowPos(window_one_past_top_left, ImGuiCond_Always, {0, 0});

    // If not set yet, let ImGui set the size
    if (state->left_side_bar_width != 0)
    {
      ImGui::SetNextWindowSize({state->left_side_bar_width, state->left_side_bar_split+1});
    }

    if (ImGui::Begin("Rules File Editor", NULL, imgui_window_flags))
    {
      do_rule_ui(&state->rule_ui, &state->loaded_rule, &state->rule_creation_thread, &state->files_loaded_state);

      state->left_side_bar_width = ImGui::GetWindowSize().x;
      state->left_side_bar_split = ImGui::GetWindowSize().y-1;
    }
    ImGui::End();

    ImGui::SetNextWindowPos(window_one_past_bottom_left, ImGuiCond_Always, {0, 1});

    // If not set yet, let ImGui set the size
    if (state->left_side_bar_width != 0)
    {
      ImGui::SetNextWindowSize({state->left_side_bar_width, screen_size.y-state->left_side_bar_split+1});
    }

    if (ImGui::Begin("Rule States Editor", NULL, imgui_window_flags))
    {
      do_named_states_ui(&state->loaded_rule.config, &state->cells_editor.active_state);

      state->left_side_bar_width = ImGui::GetWindowSize().x;
      state->left_side_bar_split = screen_size.y - (ImGui::GetWindowSize().y-1);
    }
    ImGui::End();
  }

  // Menu bar for opening and closing side bars.  Created after side bars to remove frame of flicker
  //   on change.
  r32 left_offset = state->left_side_bar_open ? state->left_side_bar_width : 0;
  r32 right_offset = state->right_side_bar_open ? state->right_side_bar_width : 0;
  if (ImGui::BeginCustomMainMenuBar({left_offset-1, 0}, {right_offset-1, 0}))
  {
    if (ImGui::MenuItem(state->left_side_bar_open ? "< Close" : "> Open"))
    {
      state->left_side_bar_open = !state->left_side_bar_open;
    }

    ImGui::SetCursorPosX(ImGui::GetWindowSize().x-60);
    if (ImGui::MenuItem(state->right_side_bar_open ? "> Close" : "< Open"))
    {
      state->right_side_bar_open = !state->right_side_bar_open;
    }

    ImGui::EndMainMenuBar();
  }

  do_files_loaded_state_ui(&state->files_loaded_state);
}
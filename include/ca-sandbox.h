#ifndef CA_SANDBOX_H_DEF
#define CA_SANDBOX_H_DEF

#include "loader.h"

#include "engine.h"
#include "opengl-buffer.h"

#include "cell-drawing.h"
#include "simulate-options-ui.h"
#include "simulation-ui.h"
#include "universe-ui.h"
#include "rule-ui.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <GL/glew.h>


#ifdef __cplusplus
extern "C" {
#endif


struct CA_SandboxState
{
  b32 init;
  u64 last_reload;

  FrameTiming frame_timing;

  ImGuiContext *imgui_context;

  GLuint general_universe_shader_program;
  GLuint general_universe_vao;
  OpenGL_Buffer general_universe_vbo;
  OpenGL_Buffer general_universe_ibo;
  GLuint general_universe_mat4_projection_matrix_uniform;

  OpenGL_Buffer general_vertex_buffer;
  OpenGL_Buffer general_index_buffer;

  GLuint cell_instance_drawing_shader_program;
  GLuint cell_instance_drawing_vao;
  CellInstancing cell_instancing;
  GLuint cell_instance_drawing_mat4_projection_matrix_uniform;
  GLuint cell_instance_drawing_cell_block_dim_uniform;
  GLuint cell_instance_drawing_cell_width_uniform;

  GLuint screen_shader_program;
  GLuint screen_vao;

  Universe universe;
  SimulateOptions simulate_options;
  CellInitialisationOptions cell_initialisation_options;
  b32 cells_file_loaded;

  Rule loaded_rule;
  b32 rule_file_loaded;
  RuleCreationThread rule_creation_thread;

  SimulationUI simulation_ui;
  UniverseUI universe_ui;
  RuleUI rule_ui;
  CellsEditor cells_editor;

  ViewPanning view_panning;
  vec2 screen_mouse_pos;
};


struct Export_VTable
{
  LoaderReturnStatus (*main_loop)(s32 argc, const char *argv[], Engine *engine, CA_SandboxState **state);
};


LoaderReturnStatus
main_loop(int argc, const char *argv[], Engine *engine, CA_SandboxState **state);


#ifdef __cplusplus
}
#endif

#endif
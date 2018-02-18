#ifndef CA_SANDBOX_H_DEF
#define CA_SANDBOX_H_DEF

#include "loader.h"

#include "engine/engine.h"
#include "engine/opengl-buffer.h"
#include "engine/opengl-general-buffers.h"

#include "ca-sandbox/cell-drawing.h"
#include "ca-sandbox/view-panning.h"
#include "ca-sandbox/cell-regions.h"
#include "ca-sandbox/cell-tools.h"
#include "ca-sandbox/screen-shader.h"
#include "ca-sandbox/files-loaded-state.h"

#include "ca-sandbox/ui/simulate-options-ui.h"
#include "ca-sandbox/ui/simulation-ui.h"
#include "ca-sandbox/ui/universe-ui.h"
#include "ca-sandbox/ui/rule-ui.h"
#include "ca-sandbox/ui/cell-regions-ui.h"
#include "ca-sandbox/ui/cell-selections-ui.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

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

  GeneralVertexBuffer general_vertex_buffer;
  GeneralIndexBuffer general_index_buffer;

  CellInstancing cell_instancing;
  CellDrawing cell_drawing;

  GLuint texture_shader_program;
  GLuint rendered_texture_uniform;
  GLuint minimap_framebuffer;
  GLuint minimap_texture;
  s32vec2 minimap_texture_size;

  ScreenShader screen_shader;

  FilesLoadedState files_loaded_state;

  Universe *universe;
  SimulateOptions simulate_options;
  CellInitialisationOptions cell_initialisation_options;

  Rule loaded_rule;
  RuleCreationThread rule_creation_thread;

  CellSelectionsUI cell_selections_ui;
  SimulationUI simulation_ui;
  UniverseUI universe_ui;
  RuleUI rule_ui;
  CellsEditor cells_editor;

  CellRegions cell_regions;
  CellRegionsUI cell_regions_ui;

  CellTools cell_tools;

  ViewPanning view_panning;
  vec2 screen_mouse_pos;

  b32 left_side_bar_open;
  b32 right_side_bar_open;
  r32 left_side_bar_width;
  r32 right_side_bar_width;
  r32 left_side_bar_split;
  r32 right_side_bar_split;
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
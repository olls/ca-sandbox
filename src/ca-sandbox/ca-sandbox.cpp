#include "ca-sandbox.h"

#include "loader.h"
#include "types.h"
#include "vectors.h"
#include "print.h"
#include "text.h"
#include "util.h"
#include "allocate.h"
#include "engine.h"
#include "opengl-util.h"
#include "opengl-shaders.h"
#include "opengl-buffer.h"
#include "opengl-general-buffers.h"
#include "view-panning.h"

#include "universe.h"
#include "load-universe.h"
#include "cell-drawing.h"
#include "rule.h"
#include "load-rule.h"
#include "simulate.h"
#include "rule-ui.h"
#include "simulation-ui.h"
#include "simulate-options-ui.h"
#include "universe-ui.h"
#include "cells-editor.h"
#include "save-universe.h"
#include "named-states-ui.h"
#include "save-rule-config.h"

#include "imgui.h"
#include "imgui_impl_sdl_gl3.h"

#include <GL/glew.h>

// #define DEBUG_CELL_BLOCK_DRAWING
// #define DEBUG_MOUSE_UNIVERSE_POSITION_DRAWING
// #define DEBUG_SCREEN_DRAWING

/// @file
/// @brief Program root file.
///


/// The number of simulation frames per second.
#ifdef GDB_DEBUG
const r32 INITIAL_SIM_FREQUENCY = 1;
#else
const r32 INITIAL_SIM_FREQUENCY = 30;
#endif


/// @brief Compile all the OpenGL shaders used in the program.
///
/// TODO: Shader compilation should probably be moved to the locations where the shaders are used.
b32
init_shaders(GLuint *general_universe_shader_program, GLuint *cell_instance_drawing_shader_program, GLuint *screen_shader_program)
{
  b32 success = true;

  GLenum shader_types[] = {
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER
  };

  const char *general_cell_block[] = {
    "shaders/general-universe.glvs",
    "shaders/screen.glfs"
  };

  success &= create_shader_program(general_cell_block, shader_types, 2, general_universe_shader_program);

  const char *cells_filenames[] = {
    "shaders/cell-instancing.glvs",
    "shaders/screen.glfs"
  };

  success &= create_shader_program(cells_filenames, shader_types, 2, cell_instance_drawing_shader_program);

  const char *screen_filenames[] = {
    "shaders/screen.glvs",
    "shaders/screen.glfs"
  };

  success &= create_shader_program(screen_filenames, shader_types, 2, screen_shader_program);

  return success;
}


static const CA_SandboxState CA_SANDBOX_INITIALISER = {
  .init = true,
  .imgui_context = 0,

  .frame_timing = {},

  .general_universe_shader_program = 0,
  .general_universe_vao = 0,
  .general_universe_vbo = {},
  .general_universe_ibo = {},
  .general_universe_mat4_projection_matrix_uniform = 0,

  .general_vertex_buffer = {},
  .general_index_buffer = {},

  .cell_instance_drawing_shader_program = 0,
  .cell_instance_drawing_vao = 0,
  .cell_instancing = {},
  .cell_instance_drawing_mat4_projection_matrix_uniform = 0,
  .cell_instance_drawing_cell_block_dim_uniform = 0,
  .cell_instance_drawing_cell_width_uniform = 0,

  .screen_shader_program = 0,
  .screen_vao = 0,

  .universe = {},
  .simulate_options = {},
  .cell_initialisation_options = {},
  .cells_file_loaded = false,

  .loaded_rule = {},
  .rule_file_loaded = false,
  .rule_creation_thread = {},

  .simulation_ui = {
    .sim_frequency = INITIAL_SIM_FREQUENCY,
    .simulating = false,
    .step_simulation = false,
    .simulation_step = 0,
    .last_sim_time = get_us(),
    .last_simulation_delta = 0
  },
  .universe_ui = {},
  .rule_ui = {},
  .cells_editor = {},

  .view_panning = {
    .scale = 0.3,
    .offset = {0,0}
  },
  .screen_mouse_pos = {}
};


#ifdef __cplusplus
extern "C" {
#endif

/// Main program setup and loop
LoaderReturnStatus
main_loop(int argc, const char *argv[], Engine *engine, CA_SandboxState **state_ptr)
{
  LoaderReturnStatus result = {
    .success = true,
    .reload = false
  };

  if (*state_ptr == 0)
  {
    *state_ptr = allocate(CA_SandboxState, 1);

    memcpy(*state_ptr, &CA_SANDBOX_INITIALISER, sizeof(CA_SandboxState));
  }

  CA_SandboxState *state = *state_ptr;

  FrameTiming *frame_timing = &state->frame_timing;

  OpenGL_Buffer *general_universe_vbo = &state->general_universe_vbo;
  OpenGL_Buffer *general_universe_ibo = &state->general_universe_ibo;
  OpenGL_Buffer *general_vertex_buffer = &state->general_vertex_buffer;
  OpenGL_Buffer *general_index_buffer = &state->general_index_buffer;

  CellInstancing *cell_instancing = &state->cell_instancing;

  Universe *universe = &state->universe;
  SimulateOptions *simulate_options = &state->simulate_options;
  CellInitialisationOptions *cell_initialisation_options = &state->cell_initialisation_options;
  Rule *loaded_rule = &state->loaded_rule;
  SimulationUI *simulation_ui = &state->simulation_ui;
  UniverseUI *universe_ui = &state->universe_ui;
  RuleUI *rule_ui = &state->rule_ui;
  CellsEditor *cells_editor = &state->cells_editor;
  RuleCreationThread *rule_creation_thread = &state->rule_creation_thread;
  ViewPanning *view_panning = &state->view_panning;

  if (state->init)
  {
    state->init = false;

    engine_setup_loop(frame_timing);

    // TODO: Ensure stuff is destroyed / freed before re-init-ing

    // Create ImGui context and font atlas
    state->imgui_context = ImGui::CreateContext();
    ImGui::SetCurrentContext(state->imgui_context);
    ImGui::GetIO().Fonts = new ImFontAtlas();

    ImGui_ImplSdlGL3_Init(engine->sdl_window);

    opengl_create_general_buffers(general_vertex_buffer, general_index_buffer);

    b32 shader_success = init_shaders(&state->general_universe_shader_program, &state->cell_instance_drawing_shader_program, &state->screen_shader_program);
    result.success &= shader_success;

    // General universe drawing
    {
      state->general_universe_mat4_projection_matrix_uniform = glGetUniformLocation(state->general_universe_shader_program, "projection_matrix");

      // Generate and Bind VAO
      glGenVertexArrays(1, &state->general_universe_vao);
      glBindVertexArray(state->general_universe_vao);

      // Generate and Bind VBO
      create_opengl_buffer(general_universe_vbo, sizeof(GeneralUnvierseVertex), GL_ARRAY_BUFFER, GL_STREAM_DRAW);
      glBindBuffer(general_universe_vbo->binding_target, general_universe_vbo->id);

      // Generate and Bind IBO
      create_opengl_buffer(general_universe_ibo, sizeof(GLushort), GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_DRAW);
      glBindBuffer(general_universe_ibo->binding_target, general_universe_ibo->id);

      opengl_print_errors();
      glBindVertexArray(0);
    }

    // Cell instance drawing
    {
      state->cell_instance_drawing_mat4_projection_matrix_uniform = glGetUniformLocation(state->cell_instance_drawing_shader_program, "projection_matrix");
      state->cell_instance_drawing_cell_block_dim_uniform = glGetUniformLocation(state->cell_instance_drawing_shader_program, "cell_block_dim");
      state->cell_instance_drawing_cell_width_uniform = glGetUniformLocation(state->cell_instance_drawing_shader_program, "cell_width");

      glGenVertexArrays(1, &state->cell_instance_drawing_vao);
      glBindVertexArray(state->cell_instance_drawing_vao);

      init_cell_drawing(cell_instancing, general_vertex_buffer, general_index_buffer);

      glBindVertexArray(0);
    }

#ifdef DEBUG_SCREEN_DRAWING
    {
      glGenVertexArrays(1, &state->screen_vao);
    }
#endif

    // Load initial filename arguments
    if (argc >= 3)
    {
      const char *rule_filename = argv[2];
      copy_string(rule_ui->file_picker.selected_file, rule_filename, strlen(rule_filename)+1); // Plus one for \0
      rule_ui->reload_rule_file = true;

      const char *cells_filename = argv[1];
      copy_string(universe_ui->cells_file_picker.selected_file, cells_filename, strlen(cells_filename)+1);
      universe_ui->reload_cells_file = true;
    }

    opengl_print_errors();
  }

  ImGui::SetCurrentContext(state->imgui_context);

  b32 running = true;
  while (running && result.success && !result.reload)
  {
    //
    // Process inputs
    //

    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
      ImGui_ImplSdlGL3_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
      {
          running = false;
      }
    }

    //
    // Start frame
    //

    engine_frame_start(frame_timing);
    ImGui_ImplSdlGL3_NewFrame(engine->sdl_window);

    ImGuiIO& io = ImGui::GetIO();

    if (io.KeysDown[SDLK_ESCAPE] ||
        (io.KeyCtrl && io.KeysDown['w']))
    {
      running = false;
    }
    if (io.KeyCtrl && ImGui::IsKeyReleased('r'))
    {
      state->last_reload = frame_timing->frame_start;
      result.reload = true;
    }

    if (ImGui::IsMousePosValid())
    {
      state->screen_mouse_pos = ImGui::GetMousePos();
      state->screen_mouse_pos = vec2_divide(state->screen_mouse_pos, vec2(io.DisplaySize));
      state->screen_mouse_pos = vec2_multiply(state->screen_mouse_pos, 2);
      state->screen_mouse_pos = vec2_subtract(state->screen_mouse_pos, 1);
      state->screen_mouse_pos.y *= -1;
    }

    s32vec2 window_size = vec2_to_s32vec2(io.DisplaySize);

    update_view_panning(view_panning, state->screen_mouse_pos);
    update_view_projection_matrix(view_panning, window_size);

    UniversePosition mouse_universe_pos = screen_position_to_universe_position(view_panning, state->screen_mouse_pos);

    //
    // Draw imGui elements
    //

    ImGui::ShowTestWindow();

    do_simulation_ui(simulation_ui, frame_timing->frame_start, loaded_rule->rule_tree_built, &universe_ui->reload_cells_file, &universe_ui->save_cells_file);
    do_rule_ui(rule_ui, loaded_rule, rule_creation_thread);
    do_simulate_options_ui(simulate_options, universe);
    do_universe_ui(universe_ui, universe, simulate_options, cell_initialisation_options, &loaded_rule->config.named_states);
    do_named_states_ui(&loaded_rule->config, &cells_editor->active_state);

    //
    // Save
    //

    if (universe_ui->save_cells_file)
    {
      universe_ui->save_cells_file = false;
      result.success &= save_universe_to_file(universe_ui->cells_file_picker.selected_file, universe, simulate_options, cell_initialisation_options, &loaded_rule->config.named_states);
    }

    if (rule_ui->save_rule_file)
    {
      rule_ui->save_rule_file = false;
      result.success &= save_rule_config_to_file(rule_ui->file_picker.selected_file, &loaded_rule->config);
    }

    //
    // Load input files
    //

    if (rule_ui->reload_rule_file)
    {
      rule_ui->reload_rule_file = false;
      state->rule_file_loaded = true;

      print("\nLoading rule file: %s\n", rule_ui->file_picker.selected_file);
      result.success &= load_rule_file(rule_ui->file_picker.selected_file, &loaded_rule->config);
      print("\n");

      // start_build_rule_tree_thread(rule_creation_thread, loaded_rule);
    }

    if (universe_ui->reload_cells_file && state->rule_file_loaded)
    {
      simulation_ui->simulating = false;
      simulation_ui->mode = Mode::EDITOR;

      universe_ui->reload_cells_file = false;
      state->cells_file_loaded = true;

      *simulate_options = default_simulation_options();
      if (cell_initialisation_options->set_of_initial_states.elements == 0)
      {
        cell_initialisation_options->set_of_initial_states.allocate_array();
      }
      default_cell_initialisation_options(cell_initialisation_options);

      result.success &= load_universe(universe_ui->cells_file_picker.selected_file, universe, simulate_options, cell_initialisation_options, &loaded_rule->config.named_states);

      if (loaded_rule->config.neighbourhood_region_size >= universe->cell_block_dim)
      {
        print("cell_block_dim is too small for the current neighbourhood_region_size.\n");
        result.success &= false;
      }
    }

    //
    // Simulate
    //

    if (simulation_ui->mode == Mode::SIMULATOR)
    {
      u32 n_simulation_steps = 0;

      if (simulation_ui->step_simulation)
      {
        simulation_ui->step_simulation = false;
        n_simulation_steps = 1;
      }
      else if (simulation_ui->simulating)
      {
        r32 sim_step_gap_us = (1000000.0 / simulation_ui->sim_frequency);
        u32 time_since_last_sim = frame_timing->frame_start - simulation_ui->last_sim_time;

        if (time_since_last_sim >= sim_step_gap_us)
        {
          n_simulation_steps = time_since_last_sim / sim_step_gap_us;
        }
      }

      if (n_simulation_steps > 0)
      {
        u64 start_sim_time = get_us();

        for (u32 simulation_step = 0;
             simulation_step < n_simulation_steps;
             ++simulation_step)
        {
          simulate_cells(simulate_options, cell_initialisation_options, loaded_rule, universe, ++simulation_ui->simulation_step);
        }

        u64 end_sim_time = get_us();
        simulation_ui->last_sim_time = end_sim_time;

        simulation_ui->last_simulation_delta = (u32)(end_sim_time - start_sim_time) * (1.0/n_simulation_steps);
      }
    }
    else if (simulation_ui->mode == Mode::EDITOR)
    {
      if (state->cells_file_loaded)
      {
        do_cells_editor(cells_editor, universe, cell_initialisation_options, &loaded_rule->config.named_states, mouse_universe_pos, view_panning->currently_panning);
      }
    }

    //
    // Render
    //

    glViewport(0, 0, window_size.x, window_size.y);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    r32 cell_width = 1;

    //
    // Cell instance drawing
    //

    upload_cell_instances(universe, cell_instancing, cells_editor);

    glBindVertexArray(state->cell_instance_drawing_vao);
    glUseProgram(state->cell_instance_drawing_shader_program);
    glUniformMatrix4fv(state->cell_instance_drawing_mat4_projection_matrix_uniform, 1, GL_TRUE, &view_panning->projection_matrix[0][0]);

    glUniform1i(state->cell_instance_drawing_cell_block_dim_uniform, universe->cell_block_dim);
    glUniform1f(state->cell_instance_drawing_cell_width_uniform, cell_width);

    // Re-initialise attributes in case instance buffer has been reallocated
    init_cell_instances_buffer_attributes(&cell_instancing->buffer, general_vertex_buffer, state->cell_instance_drawing_shader_program);

    draw_cell_instances(cell_instancing);

    opengl_print_errors();
    glBindVertexArray(0);

    //
    // General universe triangles drawing
    //

    if (cells_editor->cell_block_highlighted)
    {
      general_universe_vbo->elements_used = 0;
      general_universe_ibo->elements_used = 0;

      vec4 potential_cell_block_colour = {0.8, 0.8, 0.8, 1};

      GeneralUnvierseVertex cell_block_vertices[] = {
        {{cells_editor->highlighted_cell_block, {0, 0}}, potential_cell_block_colour},
        {{cells_editor->highlighted_cell_block, {1, 0}}, potential_cell_block_colour},
        {{cells_editor->highlighted_cell_block, {1, 1}}, potential_cell_block_colour},
        {{cells_editor->highlighted_cell_block, {0, 1}}, potential_cell_block_colour}
      };

      GLushort vbo_index_a = opengl_buffer_new_element(general_universe_vbo, cell_block_vertices + 0);
      GLushort vbo_index_b = opengl_buffer_new_element(general_universe_vbo, cell_block_vertices + 1);
      GLushort vbo_index_c = opengl_buffer_new_element(general_universe_vbo, cell_block_vertices + 2);
      GLushort vbo_index_d = opengl_buffer_new_element(general_universe_vbo, cell_block_vertices + 3);

      GLuint first_index = opengl_buffer_new_element(general_universe_ibo, &vbo_index_a);
      opengl_buffer_new_element(general_universe_ibo, &vbo_index_b);
      opengl_buffer_new_element(general_universe_ibo, &vbo_index_c);
      opengl_buffer_new_element(general_universe_ibo, &vbo_index_a);
      opengl_buffer_new_element(general_universe_ibo, &vbo_index_c);
      GLuint last_index = opengl_buffer_new_element(general_universe_ibo, &vbo_index_d);

      glBindVertexArray(state->general_universe_vao);
      glUseProgram(state->general_universe_shader_program);
      glUniformMatrix4fv(state->general_universe_mat4_projection_matrix_uniform, 1, GL_TRUE, &view_panning->projection_matrix[0][0]);

      // Get attribute locations
      init_general_universe_attributes(general_universe_vbo, state->general_universe_shader_program);

      void *general_universe_ibo_offset = (void *)(intptr_t)(first_index * sizeof(GLushort));
      glDrawElements(GL_TRIANGLES, (last_index - first_index) + 1, GL_UNSIGNED_SHORT, general_universe_ibo_offset);

      opengl_print_errors();
      glBindVertexArray(0);
    }

#ifdef DEBUG_CELL_BLOCK_DRAWING
    //
    // Debug cell blocks drawing
    //

    general_universe_vbo->elements_used = 0;
    general_universe_ibo->elements_used = 0;

    BufferDrawingLocation ibo_outlines = debug_cell_block_outline_drawing_upload(universe, general_universe_vbo, general_universe_ibo);

#ifdef DEBUG_MOUSE_UNIVERSE_POSITION_DRAWING
    GeneralUnvierseVertex origin_vertex = {{}, {1, 0, 0, 1}};
    GeneralUnvierseVertex mouse_vertex = {mouse_universe_pos, {1, 0, 0, 1}};
    GLushort origin_index = opengl_buffer_new_element(general_universe_vbo, &origin_vertex);
    GLushort mouse_index = opengl_buffer_new_element(general_universe_vbo, &mouse_vertex);
    opengl_buffer_new_element(general_universe_ibo, &origin_index);
    opengl_buffer_new_element(general_universe_ibo, &mouse_index);

    ibo_outlines.n_elements += 2;
#endif

    glBindVertexArray(state->general_universe_vao);
    glUseProgram(state->general_universe_shader_program);
    glUniformMatrix4fv(state->general_universe_mat4_projection_matrix_uniform, 1, GL_TRUE, &view_panning->projection_matrix[0][0]);

    // Get attribute locations
    init_general_universe_attributes(general_universe_vbo, state->general_universe_shader_program);

    debug_cell_block_outline_draw(general_universe_vbo, general_universe_ibo, ibo_outlines);

    opengl_print_errors();
    glBindVertexArray(0);
#endif

#ifdef DEBUG_SCREEN_DRAWING
    //
    // Debug screen rendering
    //

    glBindVertexArray(state->screen_vao);
    glUseProgram(state->screen_shader_program);
    glBindBuffer(general_vertex_buffer->binding_target, general_vertex_buffer->id);

    // Draw mouse position line
    vec2 vertices[] = {{0,  0}, state->screen_mouse_pos};
    u32 vertices_pos = opengl_buffer_add_elements(general_vertex_buffer, array_count(vertices), vertices);

    GLuint attribute_pos = glGetAttribLocation(state->screen_shader_program, "pos");
    if (attribute_pos == -1)
    {
      print("Failed to get attribute_pos\n");
    }
    glEnableVertexAttribArray(attribute_pos);
    glVertexAttribPointer(attribute_pos, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 0);

    glDrawArrays(GL_LINES, vertices_pos, array_count(vertices));

    general_vertex_buffer->elements_used -= array_count(vertices);

    opengl_print_errors();
    glUseProgram(0);
#endif

    //
    // imGUI Rendering
    //

    ImGui::Render();
    engine_swap_buffers(engine);

    engine_frame_end(frame_timing);
  }

  if (!result.reload)
  {
    ImGui_ImplSdlGL3_Shutdown();
  }

  return result;
}


Export_VTable exports = { main_loop };


#ifdef __cplusplus
}
#endif


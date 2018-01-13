#include "types.h"
#include "vectors.h"
#include "print.h"
#include "text.h"
#include "util.h"
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
    "src/shaders/general-universe.glvs",
    "src/shaders/screen.glfs"
  };

  success &= create_shader_program(general_cell_block, shader_types, 2, general_universe_shader_program);

  const char *cells_filenames[] = {
    "src/shaders/cell-instancing.glvs",
    "src/shaders/screen.glfs"
  };

  success &= create_shader_program(cells_filenames, shader_types, 2, cell_instance_drawing_shader_program);

  const char *screen_filenames[] = {
    "src/shaders/screen.glvs",
    "src/shaders/screen.glfs"
  };

  success &= create_shader_program(screen_filenames, shader_types, 2, screen_shader_program);

  return success;
}


/// Main program setup and loop
int
main(int argc, const char *argv[])
{
  Engine engine;
  b32 success = init_sdl(argc, argv, "CA Sandbox", &engine);

  engine_setup_loop(&engine);

  if (success)
  {
    GLuint general_universe_shader_program = 0;
    GLuint general_universe_vao = 0;
    OpenGL_Buffer general_universe_vbo = {};
    OpenGL_Buffer general_universe_ibo = {};
    GLuint general_universe_mat4_projection_matrix_uniform = 0;

    OpenGL_Buffer general_vertex_buffer = {};
    OpenGL_Buffer general_index_buffer = {};

    GLuint cell_instance_drawing_shader_program = 0;
    GLuint cell_instance_drawing_vao = 0;
    CellInstancing cell_instancing = {};
    GLuint cell_instance_drawing_mat4_projection_matrix_uniform = 0;
    GLuint cell_instance_drawing_cell_block_dim_uniform = 0;
    GLuint cell_instance_drawing_cell_width_uniform = 0;

    GLuint screen_shader_program = 0;
    GLuint screen_vao = 0;

    Universe universe = {};
    SimulateOptions simulate_options;
    CellInitialisationOptions cell_initialisation_options;
    b32 cells_file_loaded = false;

    Rule loaded_rule = {};
    b32 rule_file_loaded = false;

    SimulationUI simulation_ui = {
      .sim_frequency = INITIAL_SIM_FREQUENCY,
      .simulating = false,
      .step_simulation = false,
      .simulation_step = 0,
      .last_sim_time = get_us(),
      .last_simulation_delta = 0
    };

    UniverseUI universe_ui = {};
    RuleUI rule_ui = {};
    CellsEditor cells_editor = {};

    RuleCreationThread rule_creation_thread = {};

    ViewPanning view_panning = {
      .scale = 0.3,
      .offset = {0,0}
    };

    vec2 screen_mouse_pos;

    b32 init = true;
    b32 running = true;
    while (running)
    {
      if (init)
      {
        init = false;

        // TODO: Ensure stuff is destroyed / freed before re-init-ing

        ImGui_ImplSdlGL3_Init(engine.window.sdl_window);

        opengl_create_general_buffers(&general_vertex_buffer, &general_index_buffer);

        b32 shader_success = init_shaders(&general_universe_shader_program, &cell_instance_drawing_shader_program, &screen_shader_program);
        running &= shader_success;

        // General universe drawing
        {
          general_universe_mat4_projection_matrix_uniform = glGetUniformLocation(general_universe_shader_program, "projection_matrix");

          // Generate and Bind VAO
          glGenVertexArrays(1, &general_universe_vao);
          glBindVertexArray(general_universe_vao);

          // Generate and Bind VBO
          create_opengl_buffer(&general_universe_vbo, sizeof(GeneralUnvierseVertex), GL_ARRAY_BUFFER, GL_STREAM_DRAW);
          glBindBuffer(general_universe_vbo.binding_target, general_universe_vbo.id);

          // Generate and Bind IBO
          create_opengl_buffer(&general_universe_ibo, sizeof(GLushort), GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_DRAW);
          glBindBuffer(general_universe_ibo.binding_target, general_universe_ibo.id);

          opengl_print_errors();
          glBindVertexArray(0);
        }

        // Cell instance drawing
        {
          cell_instance_drawing_mat4_projection_matrix_uniform = glGetUniformLocation(cell_instance_drawing_shader_program, "projection_matrix");
          cell_instance_drawing_cell_block_dim_uniform = glGetUniformLocation(cell_instance_drawing_shader_program, "cell_block_dim");
          cell_instance_drawing_cell_width_uniform = glGetUniformLocation(cell_instance_drawing_shader_program, "cell_width");

          glGenVertexArrays(1, &cell_instance_drawing_vao);
          glBindVertexArray(cell_instance_drawing_vao);

          init_cell_drawing(&cell_instancing, &general_vertex_buffer, &general_index_buffer);

          glBindVertexArray(0);
        }

#ifdef DEBUG_SCREEN_DRAWING
        {
          glGenVertexArrays(1, &screen_vao);
        }
#endif

        // Load initial filename arguments
        if (argc >= 3)
        {
          const char *rule_filename = argv[2];
          copy_string(rule_ui.file_picker.selected_file, rule_filename, strlen(rule_filename)+1); // Plus one for \0
          rule_ui.reload_rule_file = true;

          const char *cells_filename = argv[1];
          copy_string(universe_ui.cells_file_picker.selected_file, cells_filename, strlen(cells_filename)+1);
          universe_ui.reload_cells_file = true;
        }

        if (!running)
        {
          print("Quitting\n");
          break;
        }

        opengl_print_errors();
      }

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

      engine_frame_start(&engine);
      ImGui_ImplSdlGL3_NewFrame(engine.window.sdl_window);

      ImGuiIO& io = ImGui::GetIO();

      if (io.KeysDown[SDLK_ESCAPE] ||
          (io.KeyCtrl && io.KeysDown['w']))
      {
        running = false;
      }

      if (ImGui::IsMousePosValid())
      {
        screen_mouse_pos = ImGui::GetMousePos();
        screen_mouse_pos = vec2_divide(screen_mouse_pos, vec2(io.DisplaySize));
        screen_mouse_pos = vec2_multiply(screen_mouse_pos, 2);
        screen_mouse_pos = vec2_subtract(screen_mouse_pos, 1);
        screen_mouse_pos.y *= -1;
      }

      s32vec2 window_size = vec2_to_s32vec2(io.DisplaySize);

      update_view_panning(&view_panning, screen_mouse_pos);
      update_view_projection_matrix(&view_panning, window_size);

      UniversePosition mouse_universe_pos = screen_position_to_universe_position(&view_panning, screen_mouse_pos);

      //
      // Draw imGui elements
      //

      ImGui::ShowTestWindow();

      do_simulation_ui(&simulation_ui, engine.frame_start, loaded_rule.rule_tree_built, &universe_ui.reload_cells_file);
      do_rule_ui(&rule_ui, &loaded_rule, &rule_creation_thread);
      do_simulate_options_ui(&simulate_options, &universe);
      do_universe_ui(&universe_ui, &universe, &simulate_options, &cell_initialisation_options, &loaded_rule.config.named_states);
      do_named_states_ui(&loaded_rule.config, &cells_editor.active_state);

      //
      // Save
      //

      if (simulation_ui.save_universe)
      {
        simulation_ui.save_universe = false;
        running &= save_universe_to_file(universe_ui.cells_file_picker.selected_file, &universe, &simulate_options, &loaded_rule.config.named_states);
      }

      //
      // Load input files
      //

      if (rule_ui.reload_rule_file)
      {
        rule_ui.reload_rule_file = false;
        rule_file_loaded = true;

        print("\nLoading rule file: %s\n", rule_ui.file_picker.selected_file);
        running &= load_rule_file(rule_ui.file_picker.selected_file, &loaded_rule.config);
        print("\n");

        start_build_rule_tree_thread(&rule_creation_thread, &loaded_rule);
      }

      if (universe_ui.reload_cells_file && rule_file_loaded)
      {
        simulation_ui.simulating = false;
        simulation_ui.mode = Mode::EDITOR;

        universe_ui.reload_cells_file = false;
        cells_file_loaded = true;

        simulate_options = default_simulation_options();
        if (cell_initialisation_options.set_of_initial_states.elements == 0)
        {
          cell_initialisation_options.set_of_initial_states.allocate_array();
        }
        default_cell_initialisation_options(&cell_initialisation_options);

        running &= load_universe(universe_ui.cells_file_picker.selected_file, &universe, &simulate_options, &cell_initialisation_options, &loaded_rule.config.named_states);

        if (loaded_rule.config.neighbourhood_region_size >= universe.cell_block_dim)
        {
          print("cell_block_dim is too small for the current neighbourhood_region_size.\n");
          running &= false;
        }
      }

      //
      // Simulate
      //

      if (simulation_ui.mode == Mode::SIMULATOR)
      {
        u32 n_simulation_steps = 0;

        if (simulation_ui.step_simulation)
        {
          simulation_ui.step_simulation = false;
          n_simulation_steps = 1;
        }
        else if (simulation_ui.simulating)
        {
          r32 sim_step_gap_us = (1000000.0 / simulation_ui.sim_frequency);
          u32 time_since_last_sim = engine.frame_start - simulation_ui.last_sim_time;

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
            simulate_cells(&simulate_options, &cell_initialisation_options, &loaded_rule, &universe, ++simulation_ui.simulation_step);
          }

          u64 end_sim_time = get_us();
          simulation_ui.last_sim_time = end_sim_time;

          simulation_ui.last_simulation_delta = (u32)(end_sim_time - start_sim_time) * (1.0/n_simulation_steps);
        }
      }
      else if (simulation_ui.mode == Mode::EDITOR)
      {
        if (cells_file_loaded)
        {
          do_cells_editor(&cells_editor, &universe, &cell_initialisation_options, &loaded_rule.config.named_states, mouse_universe_pos, view_panning.currently_panning);
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

      upload_cell_instances(&universe, &cell_instancing, &cells_editor);

      glBindVertexArray(cell_instance_drawing_vao);
      glUseProgram(cell_instance_drawing_shader_program);
      glUniformMatrix4fv(cell_instance_drawing_mat4_projection_matrix_uniform, 1, GL_TRUE, &view_panning.projection_matrix[0][0]);

      glUniform1i(cell_instance_drawing_cell_block_dim_uniform, universe.cell_block_dim);
      glUniform1f(cell_instance_drawing_cell_width_uniform, cell_width);

      // Re-initialise attributes in case instance buffer has been reallocated
      init_cell_instances_buffer_attributes(&cell_instancing.buffer, &general_vertex_buffer, cell_instance_drawing_shader_program);

      draw_cell_instances(&cell_instancing);

      opengl_print_errors();
      glBindVertexArray(0);

      //
      // General universe triangles drawing
      //

      if (cells_editor.cell_block_highlighted)
      {
        general_universe_vbo.elements_used = 0;
        general_universe_ibo.elements_used = 0;

        vec4 potential_cell_block_colour = {0.8, 0.8, 0.8, 1};

        GeneralUnvierseVertex cell_block_vertices[] = {
          {{cells_editor.highlighted_cell_block, {0, 0}}, potential_cell_block_colour},
          {{cells_editor.highlighted_cell_block, {1, 0}}, potential_cell_block_colour},
          {{cells_editor.highlighted_cell_block, {1, 1}}, potential_cell_block_colour},
          {{cells_editor.highlighted_cell_block, {0, 1}}, potential_cell_block_colour}
        };

        GLushort vbo_index_a = opengl_buffer_new_element(&general_universe_vbo, cell_block_vertices + 0);
        GLushort vbo_index_b = opengl_buffer_new_element(&general_universe_vbo, cell_block_vertices + 1);
        GLushort vbo_index_c = opengl_buffer_new_element(&general_universe_vbo, cell_block_vertices + 2);
        GLushort vbo_index_d = opengl_buffer_new_element(&general_universe_vbo, cell_block_vertices + 3);

        GLuint first_index = opengl_buffer_new_element(&general_universe_ibo, &vbo_index_a);
        opengl_buffer_new_element(&general_universe_ibo, &vbo_index_b);
        opengl_buffer_new_element(&general_universe_ibo, &vbo_index_c);
        opengl_buffer_new_element(&general_universe_ibo, &vbo_index_a);
        opengl_buffer_new_element(&general_universe_ibo, &vbo_index_c);
        GLuint last_index = opengl_buffer_new_element(&general_universe_ibo, &vbo_index_d);

        glBindVertexArray(general_universe_vao);
        glUseProgram(general_universe_shader_program);
        glUniformMatrix4fv(general_universe_mat4_projection_matrix_uniform, 1, GL_TRUE, &view_panning.projection_matrix[0][0]);

        // Get attribute locations
        init_general_universe_attributes(&general_universe_vbo, general_universe_shader_program);

        void *general_universe_ibo_offset = (void *)(intptr_t)(first_index * sizeof(GLushort));
        glDrawElements(GL_TRIANGLES, (last_index - first_index) + 1, GL_UNSIGNED_SHORT, general_universe_ibo_offset);

        opengl_print_errors();
        glBindVertexArray(0);
      }

#ifdef DEBUG_CELL_BLOCK_DRAWING
      //
      // Debug cell blocks drawing
      //

      general_universe_vbo.elements_used = 0;
      general_universe_ibo.elements_used = 0;

      BufferDrawingLocation ibo_outlines = debug_cell_block_outline_drawing_upload(&universe, &general_universe_vbo, &general_universe_ibo);

  #ifdef DEBUG_MOUSE_UNIVERSE_POSITION_DRAWING
      GeneralUnvierseVertex origin_vertex = {{}, {1, 0, 0, 1}};
      GeneralUnvierseVertex mouse_vertex = {mouse_universe_pos, {1, 0, 0, 1}};
      GLushort origin_index = opengl_buffer_new_element(&general_universe_vbo, &origin_vertex);
      GLushort mouse_index = opengl_buffer_new_element(&general_universe_vbo, &mouse_vertex);
      opengl_buffer_new_element(&general_universe_ibo, &origin_index);
      opengl_buffer_new_element(&general_universe_ibo, &mouse_index);

      ibo_outlines.n_elements += 2;
  #endif

      glBindVertexArray(general_universe_vao);
      glUseProgram(general_universe_shader_program);
      glUniformMatrix4fv(general_universe_mat4_projection_matrix_uniform, 1, GL_TRUE, &view_panning.projection_matrix[0][0]);

      // Get attribute locations
      init_general_universe_attributes(&general_universe_vbo, general_universe_shader_program);

      debug_cell_block_outline_draw(&general_universe_vbo, &general_universe_ibo, ibo_outlines);

      opengl_print_errors();
      glBindVertexArray(0);
#endif

#ifdef DEBUG_SCREEN_DRAWING
      //
      // Debug screen rendering
      //

      glBindVertexArray(screen_vao);
      glUseProgram(screen_shader_program);
      glBindBuffer(general_vertex_buffer.binding_target, general_vertex_buffer.id);

      // Draw mouse position line
      vec2 vertices[] = {{0,  0}, screen_mouse_pos};
      u32 vertices_pos = opengl_buffer_add_elements(&general_vertex_buffer, array_count(vertices), vertices);

      GLuint attribute_pos = glGetAttribLocation(screen_shader_program, "pos");
      if (attribute_pos == -1)
      {
        print("Failed to get attribute_pos\n");
      }
      glEnableVertexAttribArray(attribute_pos);
      glVertexAttribPointer(attribute_pos, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 0);

      glDrawArrays(GL_LINES, vertices_pos, array_count(vertices));

      general_vertex_buffer.elements_used -= array_count(vertices);

      opengl_print_errors();
      glUseProgram(0);
#endif

      //
      // imGUI Rendering
      //

      ImGui::Render();

      engine_frame_end(&engine);
    }
  }

  ImGui_ImplSdlGL3_Shutdown();

  stop_engine(&engine);
  return success;
}

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
#include "misc-ui.h"
#include "simulation-ui.h"
#include "universe-ui.h"

#include "imgui.h"
#include "imgui_impl_sdl_gl3.h"

#include <GL/glew.h>


/// @file
///
/// @brief Program root file.


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
init_shaders(GLuint *debug_cell_block_outline_drawing_shader_program, GLuint *cell_instance_drawing_shader_program, GLuint *screen_shader_program)
{
  b32 success = true;

  GLenum shader_types[] = {
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER
  };

  const char *debug_cell_block_outline_drawing_filenames[] = {
    "src/shaders/debug-cell-block-outlines.glvs",
    "src/shaders/screen.glfs"
  };

  success &= create_shader_program(debug_cell_block_outline_drawing_filenames, shader_types, 2, debug_cell_block_outline_drawing_shader_program);

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
    GLuint debug_cell_block_outline_drawing_shader_program = 0;
    OpenGL_Buffer debug_cell_block_outline_drawing_vbo = {};
    OpenGL_Buffer debug_cell_block_outline_drawing_ibo = {};
    GLuint debug_cell_block_outline_drawing_vao = 0;
    GLuint test_cell_blocks_drawing_mat4_projection_matrix_uniform = 0;

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

    MiscUI misc_ui = {
      .sim_frequency = INITIAL_SIM_FREQUENCY,
      .step_simulation = false
    };

    UniverseUI universe_ui = {};
    RuleUI rule_ui = {};

    ViewPanning view_panning = {
      .scale = 0.3,
      .offset = {0,0}
    };

    u64 last_sim_time = get_us();
    u32 last_simulation_delta = 0;

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

        b32 shader_success = init_shaders(&debug_cell_block_outline_drawing_shader_program, &cell_instance_drawing_shader_program, &screen_shader_program);
        running &= shader_success;

        // Uniforms
        test_cell_blocks_drawing_mat4_projection_matrix_uniform = glGetUniformLocation(debug_cell_block_outline_drawing_shader_program, "projection_matrix");
        cell_instance_drawing_mat4_projection_matrix_uniform = glGetUniformLocation(cell_instance_drawing_shader_program, "projection_matrix");
        cell_instance_drawing_cell_block_dim_uniform = glGetUniformLocation(cell_instance_drawing_shader_program, "cell_block_dim");
        cell_instance_drawing_cell_width_uniform = glGetUniformLocation(cell_instance_drawing_shader_program, "cell_width");

        // Debug cell block drawing
        {

          // Generate and Bind VAO
          glGenVertexArrays(1, &debug_cell_block_outline_drawing_vao);
          glBindVertexArray(debug_cell_block_outline_drawing_vao);

          // Generate and Bind VBO
          create_opengl_buffer(&debug_cell_block_outline_drawing_vbo, sizeof(s32vec2), GL_ARRAY_BUFFER, GL_STREAM_DRAW);
          glBindBuffer(debug_cell_block_outline_drawing_vbo.binding_target, debug_cell_block_outline_drawing_vbo.id);

          // Generate and Bind IBO
          create_opengl_buffer(&debug_cell_block_outline_drawing_ibo, sizeof(GLushort), GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_DRAW);
          glBindBuffer(debug_cell_block_outline_drawing_ibo.binding_target, debug_cell_block_outline_drawing_ibo.id);

          opengl_print_errors();
          glBindVertexArray(0);
        }

        // Cell instance drawing
        {
          glGenVertexArrays(1, &cell_instance_drawing_vao);
          glBindVertexArray(cell_instance_drawing_vao);

          init_cell_drawing(&cell_instancing, &general_vertex_buffer, &general_index_buffer);

          glBindVertexArray(0);
        }

        // Screen debug drawing
        {
          glGenVertexArrays(1, &screen_vao);
        }

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
        else
        {
          print("Not enough command line arguments supplied.\n");
          print("Usage: ./ca-sandbox [universe-file-path] [rule-file-path]\n\n");
          running &= false;
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

      // print("Frame Start\n");

      engine_frame_start(&engine);
      ImGui_ImplSdlGL3_NewFrame(engine.window.sdl_window);

      ImGuiIO& io = ImGui::GetIO();

      if (io.KeysDown[SDLK_ESCAPE] ||
          (io.KeyCtrl && io.KeysDown['w']))
      {
        running = false;
      }

      //
      // Draw imGui elements
      //

      // ImGui::ShowTestWindow();
      // ImGui::ShowUserGuide();
      // ImGui::ShowMetricsWindow();
      // ImGui::ShowStyleEditor();

      miscellaneous_ui(&misc_ui, last_simulation_delta);

      do_rule_ui(&rule_ui, &loaded_rule);
      simulate_ui(&simulate_options, &universe);
      do_universe_ui(&universe_ui, &universe, &simulate_options, &cell_initialisation_options, &loaded_rule.config.named_states);

      //
      // Load input files
      //

      if (rule_ui.reload_rule_file)
      {
        rule_ui.reload_rule_file = false;
        rule_file_loaded = true;

        print("\nLoading rule file: %s\n", rule_ui.file_picker.selected_file);
        running &= load_rule_file(rule_ui.file_picker.selected_file, &loaded_rule.config);

        destroy_rule_tree(&loaded_rule);
        build_rule_tree(&loaded_rule);
        // print_rule_tree(&loaded_rule);

        print("\n");
      }

      if (universe_ui.reload_cells_file && rule_file_loaded)
      {
        universe_ui.reload_cells_file = false;
        cells_file_loaded = true;

        simulate_options = default_simulation_options();
        cell_initialisation_options = default_cell_initialisation_options();

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

      if (engine.frame_start >= last_sim_time + (1000000.0 / misc_ui.sim_frequency) || misc_ui.step_simulation)
      {
        misc_ui.step_simulation = false;

        u64 start_sim_time = get_us();
        simulate_cells(&simulate_options, &cell_initialisation_options, &loaded_rule, &universe, last_sim_time);
        u64 end_sim_time = get_us();

        last_simulation_delta = end_sim_time - start_sim_time;

        last_sim_time = end_sim_time;
      }

      //
      // Render
      //

      glViewport(0, 0, engine.window.width, engine.window.height);
      glClearColor(1, 1, 1, 1);
      glClear(GL_COLOR_BUFFER_BIT);

      r32 cell_width = 1;

      s32vec2 window_size = s32vec2{(s32)engine.window.width, (s32)engine.window.height};

      vec2 screen_mouse_pos = io.MousePos;
      screen_mouse_pos = vec2_divide(screen_mouse_pos, vec2{(r32)engine.window.width, (r32)engine.window.height});
      screen_mouse_pos = vec2_multiply(screen_mouse_pos, 2);
      screen_mouse_pos = vec2_subtract(screen_mouse_pos, 1);
      screen_mouse_pos.y *= -1;

      update_view_panning(&view_panning, screen_mouse_pos);
      update_view_projection_matrix(&view_panning, window_size);

      ImGui::Text("View offset: [%f %f]", view_panning.offset.x, view_panning.offset.y);

      //
      // Cell instance drawing
      //

      upload_cell_instances(&universe, &cell_instancing);

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

#if 0
      //
      // Test cell blocks drawing
      //

      debug_cell_block_outline_drawing_upload(&universe, &debug_cell_block_outline_drawing_vbo, &debug_cell_block_outline_drawing_ibo);

      glBindVertexArray(debug_cell_block_outline_drawing_vao);
      glUseProgram(debug_cell_block_outline_drawing_shader_program);
      glUniformMatrix4fv(test_cell_blocks_drawing_mat4_projection_matrix_uniform, 1, GL_TRUE, &projection_matrix[0]);

      // Get attribute locations
      init_debug_cell_block_outline_drawing_attributes(&debug_cell_block_outline_drawing_vbo, debug_cell_block_outline_drawing_shader_program);

      debug_cell_block_outline_draw(&debug_cell_block_outline_drawing_vbo, &debug_cell_block_outline_drawing_ibo);

      opengl_print_errors();
      glBindVertexArray(0);
#endif

#if 0
      //
      // Debug screen rendering
      //

      glBindVertexArray(screen_vao);
      glUseProgram(screen_shader_program);
      glBindBuffer(general_vertex_buffer.binding_target, general_vertex_buffer.id);

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

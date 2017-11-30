#include <GL/glew.h>

#include "types.h"
#include "vectors.h"
#include "print.h"
#include "allocate.h"
#include "files.h"
#include "text.h"
#include "extendable-array.h"
#include "engine.h"
#include "opengl-util.h"
#include "opengl-shaders.h"
#include "opengl-buffer.h"
#include "opengl-general-buffers.h"

#include "cell-storage.h"
#include "load-universe.h"
#include "cell-drawing.h"
#include "rule.h"
#include "load-rule.h"
#include "simulate.h"

#include "imgui.h"
#include "imgui_impl_sdl_gl3.h"


/// @file
///
/// @brief Program root file.


/// The number of simulation frames per second.
#ifdef GDB_DEBUG
const u32 SIM_FREQUENCY = 1;
#else
const u32 SIM_FREQUENCY = 20;
#endif


/// @brief Compile all the OpenGL shaders used in the program.
///
/// TODO: Shader compilation should probably be moved to the locations where the shaders are used.
b32
init_shaders(GLuint *debug_cell_block_outline_drawing_shader_program, GLuint *cell_instance_drawing_shader_program)
{
  b32 success = true;

  const char *debug_cell_block_outline_drawing_filenames[] = {
    "src/shaders/debug-cell-block-outlines.glvs",
    "src/shaders/screen.glfs"
  };

  GLenum debug_cell_block_outline_drawing_types[] = {
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER
  };

  success &= create_shader_program(debug_cell_block_outline_drawing_filenames, debug_cell_block_outline_drawing_types, 2, debug_cell_block_outline_drawing_shader_program);

  const char *cells_filenames[] = {
    "src/shaders/cell-instancing.glvs",
    "src/shaders/screen.glfs"
  };

  GLenum cell_types[] = {
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER
  };

  success &= create_shader_program(cells_filenames, cell_types, 2, cell_instance_drawing_shader_program);

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
    Universe universe;

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

    SimulateOptions simulate_options;
    CellInitialisationOptions cell_initialisation_options;

    Rule loaded_rule = {};

    u64 last_sim_time = get_us();

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

        b32 shader_success = init_shaders(&debug_cell_block_outline_drawing_shader_program, &cell_instance_drawing_shader_program);
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

        init_cell_hashmap(&universe);

        simulate_options = default_simulation_options();
        cell_initialisation_options = default_cell_initialisation_options();

        if (argc >= 3)
        {
          const char *universe_filename = argv[1];
          print("\nLoading universe file: %s\n", universe_filename);

          File universe_file;
          String universe_file_string = get_file_string(universe_filename, &universe_file);

          running &= load_universe_from_file(universe_file_string, &universe);
          running &= load_simulate_options(universe_file_string, &simulate_options);
          running &= load_cell_initialisation_options(universe_file_string, &cell_initialisation_options);

          close_file(&universe_file);

          const char *rule_filename = argv[2];

          print("\nLoading rule file: %s\n", rule_filename);

          running &= load_rule_file(rule_filename, &loaded_rule.config);

          build_rule_tree(&loaded_rule);
          print_rule_tree(&loaded_rule);

          print("\n");
        }
        else
        {
          print("Not enough command line arguments supplied.\n");
          print("Usage: ./ca-sandbox [universe-file-path] [rule-file-path]\n\n");
          running &= false;
        }

        if (!running)
        {
          break;
        }
        else
        {
          assert(simulate_options.neighbourhood_region_size < universe.cell_block_dim);

          get_or_create_cell_block(&universe, &cell_initialisation_options, (s32vec2){0, 0});
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

      print("Frame Start\n");

      engine_frame_start(&engine);
      ImGui_ImplSdlGL3_NewFrame(engine.window.sdl_window);

      ImGuiIO& io = ImGui::GetIO();

      if (io.KeysDown[SDLK_ESCAPE] ||
          (io.KeyCtrl && io.KeysDown['w']))
      {
        running = false;
      }

      ImGui::ShowTestWindow();

      //
      // Simulate
      //

// When we are stepping through the code, we only want a maximum of one sim-frame per loop.
#ifdef GDB_DEBUG
      if (engine.frame_start >= last_sim_time + (1000000.0 / SIM_FREQUENCY))
#else
      while (engine.frame_start >= last_sim_time + (1000000.0 / SIM_FREQUENCY))
#endif
      {
        u64 start_sim_time = get_us();
        simulate_cells(&simulate_options, &cell_initialisation_options, &loaded_rule, &universe, last_sim_time);
        u64 end_sim_time = get_us();

        print("Simulation took %ldus\n", end_sim_time - start_sim_time);

        last_sim_time = end_sim_time;
      }

      //
      // Render
      //

      glViewport(0, 0, engine.window.width, engine.window.height);
      glClearColor(1, 1, 1, 1);
      glClear(GL_COLOR_BUFFER_BIT);

      r32 cell_width = 1;
      vec2 offset = {-0.05, 0.1};

      r32 view_scale = 0.05;
      r32 aspect = (r32)engine.window.height / engine.window.width;
      r32 projection_matrix[] = {
        aspect * view_scale,  0,               0,  offset.x,
        0,                   -1 * view_scale,  0,  offset.y,
        0,                    0,               1,  0,
        0,                    0,               0,  1
      };

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

      //
      // Cell instance drawing
      //

      upload_cell_instances(&universe, simulate_options.border, &cell_instancing);

      glBindVertexArray(cell_instance_drawing_vao);
      glUseProgram(cell_instance_drawing_shader_program);
      glUniformMatrix4fv(cell_instance_drawing_mat4_projection_matrix_uniform, 1, GL_TRUE, &projection_matrix[0]);

      glUniform1i(cell_instance_drawing_cell_block_dim_uniform, universe.cell_block_dim);
      glUniform1f(cell_instance_drawing_cell_width_uniform, cell_width);

      // Re-initialise attributes in case instance buffer has been reallocated
      init_cell_instances_buffer_attributes(&cell_instancing.buffer, &general_vertex_buffer, cell_instance_drawing_shader_program);

      draw_cell_instances(&cell_instancing);

      opengl_print_errors();
      glBindVertexArray(0);

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

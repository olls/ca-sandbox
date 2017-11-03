#include <GL/glew.h>

#include "types.h"
#include "vectors.h"
#include "print.h"
#include "engine.h"
#include "opengl-util.h"
#include "opengl-shaders.h"
#include "opengl-buffer.h"
#include "opengl-general-buffers.h"
#include "cell-storage.h"
#include "cell-drawing.h"
#include "simulate.h"

#include "imgui.h"
#include "imgui_impl_sdl_gl3.h"


/// @file
///
/// @brief Program root file.


/// The number of simulation frames per second.
const u32 SIM_FREQUENCEY = 5;


/// @brief Compile all the OpenGL shaders used in the program.
///
/// TODO: Shader compilation should probably be moved to the locations where the shaders are used.
b32
init_shaders(GLuint *test_cell_blocks_drawing_shader_program, GLuint *cell_instance_drawing_shader_program)
{
  b32 success = true;

  const char *blocks_filenames[] = {
    "src/shaders/debug-cell-blocks.glvs",
    "src/shaders/screen.glfs"
  };

  GLenum blocks_types[] = {
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER
  };

  success &= create_shader_program(blocks_filenames, blocks_types, 2, test_cell_blocks_drawing_shader_program);

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

    GLuint test_cell_blocks_drawing_shader_program = 0;
    OpenGL_Buffer test_cell_block_drawing_vbo = {};
    OpenGL_Buffer test_cell_block_drawing_ibo = {};
    GLuint test_cell_block_drawing_vao = 0;
    GLuint test_cell_blocks_drawing_mat4_projection_matrix_uniform = 0;

    OpenGL_Buffer general_vertex_buffer = {};
    OpenGL_Buffer general_index_buffer = {};

    GLuint cell_instance_drawing_shader_program = 0;
    GLuint cell_instance_drawing_vao = 0;
    CellInstancing cell_instancing = {};
    GLuint cell_instance_drawing_mat4_projection_matrix_uniform = 0;

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

        b32 shader_success = init_shaders(&test_cell_blocks_drawing_shader_program, &cell_instance_drawing_shader_program);
        running &= shader_success;

        // Uniforms
        test_cell_blocks_drawing_mat4_projection_matrix_uniform = glGetUniformLocation(test_cell_blocks_drawing_shader_program, "projection_matrix");
        cell_instance_drawing_mat4_projection_matrix_uniform = glGetUniformLocation(cell_instance_drawing_shader_program, "projection_matrix");


        // Debug cell block drawing
        {

          // Generate and Bind VAO
          glGenVertexArrays(1, &test_cell_block_drawing_vao);
          glBindVertexArray(test_cell_block_drawing_vao);

          // Generate and Bind VBO
          create_opengl_buffer(&test_cell_block_drawing_vbo, sizeof(s32vec2), GL_ARRAY_BUFFER, GL_STATIC_DRAW);
          glBindBuffer(test_cell_block_drawing_vbo.binding_target, test_cell_block_drawing_vbo.id);

          // Generate and Bind IBO
          create_opengl_buffer(&test_cell_block_drawing_ibo, sizeof(GLushort), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
          glBindBuffer(test_cell_block_drawing_ibo.binding_target, test_cell_block_drawing_ibo.id);

          // Get attribute locations
          GLuint attrib_location_screen_position = glGetAttribLocation(test_cell_blocks_drawing_shader_program, "s32_cell_block_position");
          glEnableVertexAttribArray(attrib_location_screen_position);
          glVertexAttribIPointer(attrib_location_screen_position, 2, GL_INT, sizeof(s32vec2), (void *)0);

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

        // NOTE: Set a seed stating state
        CellBlock *cell_block = get_cell_block(&universe, (s32vec2){0, 0});
        Cell *cell;
        #define get_cell(x, y) (cell_block->cells + ((x) * CELL_BLOCK_DIM) + (y))

        cell = get_cell(7, 7);
        cell->state = 1;
        cell = get_cell(8, 7);
        cell->state = 1;
        cell = get_cell(6, 7);
        cell->state = 1;
        cell = get_cell(7, 8);
        cell->state = 1;
        cell = get_cell(7, 6);
        cell->state = 1;

        #undef get_cell

        test_draw_cell_blocks_upload(&universe, &test_cell_block_drawing_vbo, &test_cell_block_drawing_ibo);

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

      if (engine.frame_start >= last_sim_time + 1000000*(1.0 / SIM_FREQUENCEY))
      {
        last_sim_time = engine.frame_start;
        simulate_cells(&universe, last_sim_time);
      }

      //
      // Render
      //

      glViewport(0, 0, engine.window.width, engine.window.height);
      glClearColor(1, 1, 1, 1);
      glClear(GL_COLOR_BUFFER_BIT);

      r32 aspect = (r32)engine.window.height / engine.window.width;
      r32 projection_matrix[] = {
        aspect,  0,  0,  0,
        0,      -1,  0,  0,
        0,       0,  1,  0,
        0,       0,  0,  1
      };

#if 0
      //
      // Test cell blocks drawing
      //

      glUseProgram(test_cell_blocks_drawing_shader_program);
      glUniformMatrix4fv(test_cell_blocks_drawing_mat4_projection_matrix_uniform, 1, GL_TRUE, &projection_matrix[0]);

      test_cell_block_drawing_vbo.elements_used = 0;
      test_cell_block_drawing_ibo.elements_used = 0;
      test_draw_cell_blocks_upload(&universe, &test_cell_block_drawing_vbo, &test_cell_block_drawing_ibo);

      test_draw_cell_blocks(test_cell_block_drawing_vao, &test_cell_block_drawing_vbo, &test_cell_block_drawing_ibo);

      opengl_print_errors();
#endif

      //
      // Cell instance drawing
      //

      upload_cell_instances(&universe, &cell_instancing);

      glBindVertexArray(cell_instance_drawing_vao);
      glUseProgram(cell_instance_drawing_shader_program);
      glUniformMatrix4fv(cell_instance_drawing_mat4_projection_matrix_uniform, 1, GL_TRUE, &projection_matrix[0]);

      // Re-initialise attributes in case instance buffer has been reallocated
      init_cell_instances_buffer_attributes(&cell_instancing.buffer, &general_index_buffer, &general_vertex_buffer, cell_instance_drawing_shader_program);

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

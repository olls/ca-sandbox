#include <GL/glew.h>

#include "imgui.h"
#include "imgui_impl_sdl_gl3.h"

#include "types.h"
#include "vectors.h"
#include "print.h"
#include "engine.h"
#include "opengl-util.h"
#include "opengl-shaders.h"
#include "opengl-buffer.h"
#include "cell-storage.h"
#include "cell-drawing.h"


b32
init_shaders(GLuint *test_cell_drawing_shader_program)
{
  b32 success = true;

  const char *filenames[] = {
    "src/shaders/debug-cell-blocks.glvs",
    "src/shaders/screen.glfs"
  };

  GLenum types[] = {
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER
  };

  success &= create_shader_program(filenames, types, 2, test_cell_drawing_shader_program);

  return success;
}


int
main(int argc, const char *argv[])
{
  Engine engine;
  b32 success = init_sdl(argc, argv, "Cellular Automata", &engine);

  engine_setup_loop(&engine);

  if (success)
  {
    Universe universe;

    GLuint test_cell_drawing_shader_program = 0;
    OpenGL_Buffer test_cell_drawing_vbo;
    OpenGL_Buffer test_cell_drawing_ibo;
    GLuint test_cell_drawing_vao;
    GLuint mat4_projection_matrix_uniform;

    b32 init = true;
    b32 running = true;
    while (running)
    {
      if (init)
      {
        init = false;

        ImGui_ImplSdlGL3_Init(engine.sdl_window.window);

        b32 shader_success = init_shaders(&test_cell_drawing_shader_program);
        running &= shader_success;

        // Uniforms
        mat4_projection_matrix_uniform = glGetUniformLocation(test_cell_drawing_shader_program, "projection_matrix");

        // Generate and Bind VAO
        glGenVertexArrays(1, &test_cell_drawing_vao);
        glBindVertexArray(test_cell_drawing_vao);

        // Generate and Bind VBO
        create_opengl_buffer(&test_cell_drawing_vbo, sizeof(s32Vec2), GL_ARRAY_BUFFER, GL_STATIC_DRAW);
        glBindBuffer(test_cell_drawing_vbo.binding_target, test_cell_drawing_vbo.id);

        // Generate and Bind IBO
        create_opengl_buffer(&test_cell_drawing_ibo, sizeof(GLushort), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
        glBindBuffer(test_cell_drawing_ibo.binding_target, test_cell_drawing_ibo.id);

        // Get attribute locations
        GLuint attrib_location_screen_position = glGetAttribLocation(test_cell_drawing_shader_program, "s32_cell_block_position");
        glEnableVertexAttribArray(attrib_location_screen_position);
        glVertexAttribIPointer(attrib_location_screen_position, 2, GL_INT, sizeof(s32Vec2), (void *)0);

        opengl_print_errors();
        glBindVertexArray(0);


        init_cell_hashmap(&universe);

        CellBlock *cell_block_a = get_cell_block(&universe, (s32Vec2){0, 0});
        CellBlock *cell_block_b = get_cell_block(&universe, (s32Vec2){0, 1});
        CellBlock *cell_block_c = get_cell_block(&universe, (s32Vec2){1, 0});
        CellBlock *cell_block_d = get_cell_block(&universe, (s32Vec2){1, 1});

        test_draw_cell_blocks_upload(&universe, &test_cell_drawing_vbo, &test_cell_drawing_ibo);

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
      ImGui_ImplSdlGL3_NewFrame(engine.sdl_window.window);

      ImGuiIO& io = ImGui::GetIO();

      if (io.KeysDown[SDLK_ESCAPE] ||
          (io.KeyCtrl && io.KeysDown['w']))
      {
        running = false;
      }

      ImGui::ShowTestWindow();

      //
      // Render
      //

      glViewport(0, 0, engine.sdl_window.width, engine.sdl_window.height);
      glClearColor(1, 1, 1, 1);
      glClear(GL_COLOR_BUFFER_BIT);

      r32 aspect = (r32)engine.sdl_window.height / engine.sdl_window.width;
      r32 projection_matrix[] = {
        aspect,  0,  0,  0,
        0,      -1,  0,  0,
        0,       0,  1,  0,
        0,       0,  0,  1
      };

      glUseProgram(test_cell_drawing_shader_program);
      glUniformMatrix4fv(mat4_projection_matrix_uniform, 1, GL_TRUE, &projection_matrix[0]);

      test_draw_cell_blocks(test_cell_drawing_vao, &test_cell_drawing_vbo, &test_cell_drawing_ibo);

      opengl_print_errors();
      glBindVertexArray(0);

      ImGui::Render();

      engine_frame_end(&engine);
    }
  }

  ImGui_ImplSdlGL3_Shutdown();

  stop_engine(&engine);
  return success;
}

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
init_shaders(GLuint *shader_program)
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

  success &= create_shader_program(filenames, types, 2, shader_program);

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
    GLuint shader_program = 0;
    OpenGL_Buffer test_cell_drawing_vbo;
    GLuint vao;

    b32 init = true;
    b32 running = true;
    while (running)
    {
      if (init)
      {
        init = false;

        ImGui_ImplSdlGL3_Init(engine.sdl_window.window);

        b32 shader_success = init_shaders(&shader_program);
        running &= shader_success;


        // Generate, Bind VAO
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // Generate VBO
        create_opengl_buffer(&test_cell_drawing_vbo, sizeof(s32Vec2), GL_ARRAY_BUFFER, GL_STATIC_DRAW);

        // Bind VBO
        glBindBuffer(test_cell_drawing_vbo.binding_target, test_cell_drawing_vbo.id);

        // Get attribute locations
        GLuint attrib_location_screen_position = glGetAttribLocation(shader_program, "s32_cell_block_position");
        glEnableVertexAttribArray(attrib_location_screen_position);
        glVertexAttribIPointer(attrib_location_screen_position, 2, GL_INT, sizeof(s32Vec2), (void *)0);

        glBindVertexArray(0);
        glBindBuffer(test_cell_drawing_vbo.binding_target, 0);
        opengl_print_errors();


        init_cell_hashmap(&universe);

        CellBlock *cell_block_a = get_cell_block(&universe, (s32Vec2){0, 0});
        CellBlock *cell_block_b = get_cell_block(&universe, (s32Vec2){0, 1});
        CellBlock *cell_block_c = get_cell_block(&universe, (s32Vec2){1, 0});
        CellBlock *cell_block_d = get_cell_block(&universe, (s32Vec2){1, 1});

        test_draw_cells_upload(&universe, &test_cell_drawing_vbo);

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

      test_draw_cells(shader_program, vao, &test_cell_drawing_vbo);

      opengl_print_errors();

      glBindVertexArray(0);
      glBindBuffer(test_cell_drawing_vbo.binding_target, 0);

      ImGui::Render();

      engine_frame_end(&engine);
    }
  }

  ImGui_ImplSdlGL3_Shutdown();

  stop_engine(&engine);
  return success;
}

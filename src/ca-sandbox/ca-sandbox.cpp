#include "ca-sandbox/ca-sandbox.h"

#include "loader.h"

#include "engine/types.h"
#include "engine/vectors.h"
#include "engine/print.h"
#include "engine/text.h"
#include "engine/util.h"
#include "engine/allocate.h"
#include "engine/engine.h"
#include "engine/opengl-util.h"
#include "engine/opengl-shaders.h"
#include "engine/opengl-buffer.h"
#include "engine/opengl-general-buffers.h"
#include "engine/colour.h"
#include "engine/drawing.h"

#include "ca-sandbox/cell-blocks.h"
#include "ca-sandbox/load-universe.h"
#include "ca-sandbox/cell-drawing.h"
#include "ca-sandbox/rule.h"
#include "ca-sandbox/load-rule.h"
#include "ca-sandbox/simulate.h"
#include "ca-sandbox/rule-ui.h"
#include "ca-sandbox/simulation-ui.h"
#include "ca-sandbox/simulate-options-ui.h"
#include "ca-sandbox/universe-ui.h"
#include "ca-sandbox/view-panning.h"
#include "ca-sandbox/cells-editor.h"
#include "ca-sandbox/save-universe.h"
#include "ca-sandbox/named-states-ui.h"
#include "ca-sandbox/save-rule-config.h"
#include "ca-sandbox/cell-regions-ui.h"
#include "ca-sandbox/minimap.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl_gl3.h"

#include <GL/glew.h>

// #define DEBUG_CELL_BLOCK_DRAWING
// #define DEBUG_MOUSE_UNIVERSE_POSITION_DRAWING
// #define DEBUG_SCREEN_DRAWING
// #define DEBUG_CELL_SELECTIONS_DRAWING

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
init_shaders(GLuint *general_universe_shader_program, GLuint *cell_instance_drawing_shader_program, GLuint *texture_shader_program, GLuint *screen_shader_program)
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

  const char *texture_filenames[] = {
    "shaders/passthrough.glvs",
    "shaders/texture.glfs"
  };

  success &= create_shader_program(texture_filenames, shader_types, 2, texture_shader_program);

  const char *screen_filenames[] = {
    "shaders/screen.glvs",
    "shaders/screen.glfs"
  };

  success &= create_shader_program(screen_filenames, shader_types, 2, screen_shader_program);

  return success;
}


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

    CA_SandboxState *state = *state_ptr;
    memset(state, 0, sizeof(CA_SandboxState));

    state->init = true;
  }

  CA_SandboxState *state = *state_ptr;

  FrameTiming *frame_timing = &state->frame_timing;

  OpenGL_Buffer *general_universe_vbo = &state->general_universe_vbo;
  OpenGL_Buffer *general_universe_ibo = &state->general_universe_ibo;
  OpenGL_Buffer *general_vertex_buffer = &state->general_vertex_buffer;
  OpenGL_Buffer *general_index_buffer = &state->general_index_buffer;

  CellInstancing *cell_instancing = &state->cell_instancing;
  CellDrawing *cell_drawing = &state->cell_drawing;

  SimulateOptions *simulate_options = &state->simulate_options;
  CellInitialisationOptions *cell_initialisation_options = &state->cell_initialisation_options;
  Rule *loaded_rule = &state->loaded_rule;
  SimulationUI *simulation_ui = &state->simulation_ui;
  UniverseUI *universe_ui = &state->universe_ui;
  RuleUI *rule_ui = &state->rule_ui;
  CellsEditor *cells_editor = &state->cells_editor;
  CellRegions *cell_regions = &state->cell_regions;
  CellRegionsUI *cell_regions_ui = &state->cell_regions_ui;
  RuleCreationThread *rule_creation_thread = &state->rule_creation_thread;
  ViewPanning *view_panning = &state->view_panning;
  CellSelectionsUI *cell_selections_ui = &state->cell_selections_ui;

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

    b32 shader_success = init_shaders(&state->general_universe_shader_program, &cell_drawing->shader_program, &state->texture_shader_program, &state->screen_shader_program);
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
      result.success &= init_cell_drawing_shaders(cell_drawing);
      init_cell_drawing(cell_drawing, cell_instancing, general_vertex_buffer, general_index_buffer);
    }

    // Minimap
    {
      state->minimap_framebuffer = create_minimap_framebuffer();
      state->rendered_texture_uniform = glGetUniformLocation(state->texture_shader_program, "rendered_texture");
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

    simulation_ui->sim_frequency = INITIAL_SIM_FREQUENCY;
    view_panning->scale = 0.3;
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
      vec2 half_screen = vec2_multiply(vec2(io.DisplaySize), 0.5);
      state->screen_mouse_pos = ImGui::GetMousePos();
      state->screen_mouse_pos = vec2_subtract(state->screen_mouse_pos, half_screen);
      state->screen_mouse_pos = vec2_divide(state->screen_mouse_pos, half_screen);
      state->screen_mouse_pos.y *= -1;
    }

    b32 mouse_click_consumed = io.WantCaptureMouse;

    s32vec2 window_size = vec2_to_s32vec2(io.DisplaySize);

    mat4x4 aspect_ratio;
    mat4x4Identity(aspect_ratio);
    aspect_ratio[0][0] = (r32)window_size.y / window_size.x;

    update_view_panning(view_panning, state->screen_mouse_pos, aspect_ratio, &mouse_click_consumed);
    update_view_projection_matrix(view_panning, aspect_ratio);

    UniversePosition mouse_universe_pos = screen_position_to_universe_position(view_panning, state->screen_mouse_pos);

    //
    // Draw imGui elements
    //

    ImGui::ShowTestWindow();

    do_cell_selections_ui(cell_selections_ui, mouse_universe_pos, &mouse_click_consumed);
    do_simulation_ui(simulation_ui, frame_timing->frame_start, loaded_rule->rule_tree_built, &universe_ui->reload_cells_file, &universe_ui->save_cells_file);
    do_rule_ui(rule_ui, loaded_rule, rule_creation_thread);
    do_simulate_options_ui(simulate_options, state->universe);
    do_universe_ui(universe_ui, &state->universe, simulate_options, cell_initialisation_options, &loaded_rule->config.named_states);
    do_named_states_ui(&loaded_rule->config, &cells_editor->active_state);
    do_cell_regions_ui(cell_regions_ui, cell_regions, state->universe, cell_selections_ui);

    if (cell_regions_ui->make_new_region)
    {
      cell_regions_ui->make_new_region = false;
      make_new_region(cell_regions, cell_selections_ui, state->universe, cell_regions_ui->new_region_name_buffer, state->minimap_framebuffer, cell_drawing, cell_instancing, general_vertex_buffer);
    }

    //
    // Save
    //

    if (universe_ui->save_cells_file && state->universe != 0)
    {
      universe_ui->save_cells_file = false;
      result.success &= save_universe_to_file(universe_ui->loaded_file_name, state->universe, simulate_options, cell_initialisation_options, &loaded_rule->config.named_states);
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

      start_build_rule_tree_thread(rule_creation_thread, loaded_rule);
    }

    if (universe_ui->reload_cells_file && state->rule_file_loaded)
    {
      simulation_ui->simulating = false;
      simulation_ui->mode = Mode::EDITOR;

      universe_ui->reload_cells_file = false;
      state->cells_file_loaded = true;

      *simulate_options = default_simulation_options();
      Array::clear(cell_initialisation_options->set_of_initial_states);
      default_cell_initialisation_options(cell_initialisation_options);

      universe_ui->loading_error_message.n_elements = 0;
      Universe *new_universe = load_universe(universe_ui->cells_file_picker.selected_file, simulate_options, cell_initialisation_options, &loaded_rule->config.named_states, universe_ui->loading_error_message);

      if (new_universe == 0)
      {
        universe_ui->loading_error = true;
      }
      else if (loaded_rule->config.neighbourhood_region_size >= new_universe->cell_block_dim)
      {
        universe_ui->loading_error = true;
        append_string(universe_ui->loading_error_message, new_string("cell_block_dim is too small for the current neighbourhood_region_size.\n"));
      }

      if (universe_ui->loading_error)
      {
        print("%.*s\n", universe_ui->loading_error_message.n_elements, universe_ui->loading_error_message.elements);
      }
      else
      {
        // Successfully loaded new_universe!
        if (state->universe)
        {
          destroy_cell_hashmap(state->universe);
          un_allocate(state->universe);
        }

        state->universe = new_universe;
        universe_ui->edited_cell_block_dim = state->universe->cell_block_dim;
        copy_string(universe_ui->loaded_file_name,
                    universe_ui->cells_file_picker.selected_file,
                    strlen(universe_ui->cells_file_picker.selected_file)+1);
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
          simulate_cells(simulate_options, cell_initialisation_options, loaded_rule, state->universe, ++simulation_ui->simulation_step);
        }

        u64 end_sim_time = get_us();
        simulation_ui->last_sim_time = end_sim_time;

        simulation_ui->last_simulation_delta = (u32)(end_sim_time - start_sim_time) * (1.0/n_simulation_steps);

        simulation_ui->simulation_delta_cumulative_average = (((u32)(end_sim_time - start_sim_time) +
                                                               (simulation_ui->simulation_delta_cumulative_average_n *
                                                                simulation_ui->simulation_delta_cumulative_average)) /
                                                              (simulation_ui->simulation_delta_cumulative_average_n + n_simulation_steps));
        simulation_ui->simulation_delta_cumulative_average_n += n_simulation_steps;
      }
    }
    else if (simulation_ui->mode == Mode::EDITOR)
    {
      if (state->cells_file_loaded)
      {
        do_cells_editor(cells_editor, state->universe, cell_initialisation_options, &loaded_rule->config.named_states, mouse_universe_pos, &mouse_click_consumed);
      }
    }

    //
    // Render
    //

    glViewport(0, 0, window_size.x, window_size.y);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    //
    // Cell instance drawing
    //

    if (state->universe != 0)
    {
      upload_cell_instances(state->universe, cell_instancing);

      draw_cell_blocks(state->universe, cell_instancing, cell_drawing, general_vertex_buffer, view_panning->projection_matrix);
    }

    //
    // General universe triangles drawing
    //

    glBindVertexArray(state->general_universe_vao);
    glUseProgram(state->general_universe_shader_program);
    mat4x4 projection_matrix_t;
    mat4x4Transpose(projection_matrix_t, view_panning->projection_matrix);
    glUniformMatrix4fv(state->general_universe_mat4_projection_matrix_uniform, 1, GL_TRUE, &projection_matrix_t[0][0]);

    general_universe_vbo->elements_used = 0;
    general_universe_ibo->elements_used = 0;

    BufferDrawingLocation general_universe_triangles_ibo = {};
    general_universe_triangles_ibo.start_position = general_universe_ibo->elements_used;

    if (cells_editor->cell_highlighted)
    {
      vec4 highlighted_cell_colour = get_state_colour(cells_editor->highlighted_cell_state);
      highlighted_cell_colour = lighten_colour(highlighted_cell_colour);
      GeneralUnvierseVertex colour_template = {{{}, {}}, highlighted_cell_colour};

      u32 colour_template_position_offset = offsetof(GeneralUnvierseVertex, vertex);

      r32 cell_dim = 1.0 / state->universe->cell_block_dim;
      UniversePosition cell_size = {{}, {cell_dim, cell_dim}};

      general_universe_triangles_ibo.n_elements += make_square_triangle_vertices(cells_editor->highlighted_cell, cell_size, colour_template, colour_template_position_offset, general_universe_vbo, general_universe_ibo);
    }

    if (cells_editor->cell_block_highlighted)
    {
      vec4 potential_cell_block_colour = {0.8, 0.8, 0.8, 1};
      GeneralUnvierseVertex colour_template = {{{}, {}}, potential_cell_block_colour};

      u32 colour_template_position_offset = offsetof(GeneralUnvierseVertex, vertex);

      UniversePosition block_size = {{1, 1}, {}};
      UniversePosition block_start_pos = {cells_editor->highlighted_cell_block, {}};
      general_universe_triangles_ibo.n_elements += make_square_triangle_vertices(block_start_pos, block_size, colour_template, colour_template_position_offset, general_universe_vbo, general_universe_ibo);
    }

    if (cell_selections_ui->making_selection || cell_selections_ui->selection_made)
    {
      general_universe_triangles_ibo.n_elements += cell_selections_drawing_upload(cell_selections_ui, state->universe, general_universe_vbo, general_universe_ibo);
    }

    // Get attribute locations
    init_general_universe_attributes(general_universe_vbo, state->general_universe_shader_program);

    glDrawElements(GL_TRIANGLES, general_universe_triangles_ibo.n_elements, GL_UNSIGNED_SHORT, (void *)(intptr_t)general_universe_triangles_ibo.start_position);

    opengl_print_errors();
    glBindVertexArray(0);

    //
    // Mini map drawing
    //

    if (state->universe != 0)
    {

      if (vec2_eq(state->minimap_texture_size, {0, 0}))
      {
        state->minimap_texture_size = {300, 300};
        state->minimap_texture = create_minimap_texture(state->minimap_texture_size, state->minimap_framebuffer);
      }

      ImGui::Value("tex", state->minimap_texture);

      draw_minimap_texture(state->universe, cell_instancing, cell_drawing, general_vertex_buffer, state->minimap_framebuffer, state->minimap_texture, state->minimap_texture_size);
      opengl_print_errors();

      draw_minimap_texture_to_screen(state->minimap_framebuffer, state->minimap_texture, state->minimap_texture_size, state->texture_shader_program, state->rendered_texture_uniform);

      // Reset view port (draw_minimap_texture_to_screen modifies it...)
      glViewport(0, 0, window_size.x, window_size.y);

      opengl_print_errors();
    }

    //
    // Debug universe lines drawing
    //

    glBindVertexArray(state->general_universe_vao);
    glUseProgram(state->general_universe_shader_program);
    glUniformMatrix4fv(state->general_universe_mat4_projection_matrix_uniform, 1, GL_TRUE, &projection_matrix_t[0][0]);

    general_universe_vbo->elements_used = 0;
    general_universe_ibo->elements_used = 0;
    BufferDrawingLocation debug_universe_lines_ibo = {};

#ifdef DEBUG_CELL_BLOCK_DRAWING
    if (state->universe != 0)
    {
      debug_universe_lines_ibo.n_elements += debug_cell_block_outline_drawing_upload(state->universe, general_universe_vbo, general_universe_ibo);
    }
#endif

#ifdef DEBUG_MOUSE_UNIVERSE_POSITION_DRAWING
    GeneralUnvierseVertex origin_vertex = {{}, {1, 0, 0, 1}};
    GeneralUnvierseVertex mouse_vertex = {mouse_universe_pos, {1, 0, 0, 1}};
    GLushort origin_index = opengl_buffer_new_element(general_universe_vbo, &origin_vertex);
    GLushort mouse_index = opengl_buffer_new_element(general_universe_vbo, &mouse_vertex);
    opengl_buffer_new_element(general_universe_ibo, &origin_index);
    opengl_buffer_new_element(general_universe_ibo, &mouse_index);

    debug_universe_lines_ibo.n_elements += 2;
#endif

#ifdef DEBUG_CELL_SELECTIONS_DRAWING
    if (state->universe != 0)
    {
      debug_universe_lines_ibo.n_elements += debug_cell_selections_drawing_upload(cell_selections_ui, state->universe, general_universe_vbo, general_universe_ibo);
    }
#endif

    // Get attribute locations
    init_general_universe_attributes(general_universe_vbo, state->general_universe_shader_program);

    debug_lines_draw(general_universe_vbo, general_universe_ibo, debug_universe_lines_ibo);

    opengl_print_errors();
    glBindVertexArray(0);

    //
    // Debug screen rendering
    //

#ifdef DEBUG_SCREEN_DRAWING
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


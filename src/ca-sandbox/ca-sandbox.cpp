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
#include "ca-sandbox/view-panning.h"
#include "ca-sandbox/cells-editor.h"
#include "ca-sandbox/save-universe.h"
#include "ca-sandbox/save-rule-config.h"
#include "ca-sandbox/minimap.h"
#include "ca-sandbox/main-gui.h"

#include "ca-sandbox/ui/cell-selections-ui.h"

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
init_shaders(GLuint *general_universe_shader_program, GLuint *cell_instance_drawing_shader_program, GLuint *texture_shader_program)
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

  GeneralVertexBuffer *general_vertex_buffer = &state->general_vertex_buffer;
  GeneralIndexBuffer *general_index_buffer = &state->general_index_buffer;

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
  CellTools *cell_tools = &state->cell_tools;
  ScreenShader *screen_shader = &state->screen_shader;
  FilesLoadedState *files_loaded_state = &state->files_loaded_state;

  if (state->init)
  {
    state->init = false;

    engine_setup_loop(frame_timing);

    // TODO: Ensure stuff is destroyed / freed before re-init-ing

    // Create ImGui context and font atlas
    state->imgui_context = ImGui::CreateContext();
    ImGui::SetCurrentContext(state->imgui_context);

    ImGuiIO& io = ImGui::GetIO();
    ImFont *font = io.Fonts->AddFontFromFileTTF("fonts/Roboto-Regular.ttf", 24);
    font->Scale = .7;

    ImGui_ImplSdlGL3_Init(engine->sdl_window);

    opengl_create_general_buffers(general_vertex_buffer, general_index_buffer);

    b32 shader_success = init_shaders(&state->general_universe_shader_program, &cell_drawing->shader_program, &state->texture_shader_program);
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

    // Screen space rendering
    {
      result.success &= initialise_screen_shader(screen_shader);
    }

    // Load initial filename arguments
    if (argc >= 3)
    {
      String rule_filename = new_string(argv[2]);
      append_string(rule_ui->file_picker.selected_file, rule_filename);
      flag_load_rule_file(files_loaded_state);

      String cells_filename = new_string(argv[1]);
      append_string(universe_ui->cells_file_picker.selected_file, cells_filename);

      // NOTE: Setting value directly here instead of using flag_load_cells_file() because the rule
      //         file is not loaded until we get to in the main loop.
      files_loaded_state->load_cells_file = true;
    }

    opengl_print_errors();

    setup_imgui_style();

    simulation_ui->sim_frequency = INITIAL_SIM_FREQUENCY;
    view_panning->scale = 0.3;
    state->left_side_bar_open = true;
    state->right_side_bar_open = true;
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

    if (io.KeyCtrl && ImGui::IsKeyDown(SDL_SCANCODE_W))
    {
      running = false;
    }
    if (io.KeyCtrl && ImGui::IsKeyReleased(SDL_SCANCODE_R))
    {
      state->last_reload = frame_timing->frame_start;
      result.reload = true;
    }

    if (!ImGui::IsMousePosValid())
    {
      state->screen_mouse_pos = {-1, -1};
    }
    else
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

    do_cell_selections_ui(cell_selections_ui, mouse_universe_pos, &mouse_click_consumed);

    //
    // Keyboard short-cuts
    //

    if (!io.WantCaptureKeyboard)
    {
      if (ImGui::IsKeyPressed(SDL_SCANCODE_DELETE))
      {
        cell_tools->flags |= CellToolFlags__SetSelectionNull;
      }
    }

    //
    // Draw imGui elements
    //


    do_main_gui(state, window_size);

    update_cell_regions(cell_regions_ui, cell_regions, cell_selections_ui, state->universe, state->minimap_framebuffer, cell_drawing, cell_instancing, general_vertex_buffer, mouse_universe_pos, &mouse_click_consumed);

    perform_cell_tools(cell_tools, cell_selections_ui, state->universe, &loaded_rule->config);

    // TODO:  Where does this belong?
    if (ImGui::Button("Centre Universe"))
    {
      centre_universe(view_panning, state->universe, window_size);
    }

    //
    // Save files
    //

    if (universe_ui->save_cells_file && state->universe != 0)
    {
      universe_ui->save_cells_file = false;
      delete_null_cell_blocks(state->universe, &loaded_rule->config);
      result.success &= save_universe_to_file(universe_ui->loaded_file_name, state->universe, simulate_options, cell_initialisation_options, &loaded_rule->config.named_states);
    }

    if (rule_ui->save_rule_file)
    {
      rule_ui->save_rule_file = false;
      result.success &= save_rule_config_to_file(rule_ui->file_picker.selected_file.elements, &loaded_rule->config);
    }

    //
    // Close files
    //

    if (files_loaded_state->unload_cells_file)
    {
      files_loaded_state->unload_cells_file = false;
      files_loaded_state->cells_file_loaded = false;
    }

    if (files_loaded_state->unload_rule_file)
    {
      files_loaded_state->unload_rule_file = false;
      files_loaded_state->rule_file_loaded = false;
    }

    //
    // Load input files
    //

    if (files_loaded_state->load_rule_file)
    {
      if (rule_ui->file_picker.selected_file.n_elements == 0)
      {
        // TODO: User error message
        printf("Error: no filename selected\n");
        assert(0);
      }
      else
      {
        files_loaded_state->load_rule_file = false;
        files_loaded_state->rule_file_loaded = true;

        char *loading_file_name = dynamic_string_to_heap(rule_ui->file_picker.selected_file);

        print("\nLoading rule file: %s\n", loading_file_name);
        result.success &= load_rule_file(loading_file_name, &loaded_rule->config);
        print("\n");

        un_allocate(loading_file_name);

        start_build_rule_tree_thread(rule_creation_thread, loaded_rule);
      }
    }


    if (files_loaded_state->load_cells_file)
    {
      files_loaded_state->load_cells_file = false;

      if (universe_ui->new_universe_ui.create_new_universe)
      {
        char *loading_file_name = dynamic_string_to_heap(universe_ui->new_universe_ui.directory_picker.selected_file);

        if (state->universe)
        {
          destroy_cell_hashmap(state->universe);
          un_allocate(state->universe);
          state->universe = 0;
        }

        state->universe = allocate(Universe, 1);
        init_cell_hashmap(state->universe);

        *simulate_options = default_simulation_options();
        Array::clear(cell_initialisation_options->set_of_initial_states);
        default_cell_initialisation_options(cell_initialisation_options);

        state->universe->cell_block_dim = universe_ui->new_universe_ui.cell_block_dim;
        universe_ui->edited_cell_block_dim = state->universe->cell_block_dim;

        files_loaded_state->cells_file_loaded = true;
        simulation_ui->simulating = false;
        simulation_ui->mode = Mode::Editor;

        strcpy(universe_ui->loaded_file_name, loading_file_name);
        un_allocate(loading_file_name);
      }
      else
      {
        simulation_ui->simulating = false;
        simulation_ui->mode = Mode::Editor;

        *simulate_options = default_simulation_options();
        Array::clear(cell_initialisation_options->set_of_initial_states);
        default_cell_initialisation_options(cell_initialisation_options);

        char *loading_file_name = dynamic_string_to_heap(universe_ui->cells_file_picker.selected_file);

        universe_ui->loading_error_message.n_elements = 0;
        Universe *new_universe = load_universe(loading_file_name, simulate_options, cell_initialisation_options, &loaded_rule->config.named_states, universe_ui->loading_error_message);

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
            state->universe = 0;
          }

          state->universe = new_universe;
          universe_ui->edited_cell_block_dim = state->universe->cell_block_dim;

          strcpy(universe_ui->loaded_file_name, loading_file_name);
        }

        un_allocate(loading_file_name);
      }

      if (!universe_ui->loading_error)
      {
        centre_universe(view_panning, state->universe, window_size);
      }

      files_loaded_state->cells_file_loaded = !universe_ui->loading_error;
    }

    //
    // Simulate
    //

    if (simulation_ui->mode == Mode::Simulator)
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
        simulation_ui->last_simulation_delta_per_cell_block = simulation_ui->last_simulation_delta * (1.0/state->universe->n_cell_blocks_in_use);


        simulation_ui->simulation_delta_cumulative_average = (((u32)(end_sim_time - start_sim_time) +
                                                               (simulation_ui->simulation_delta_cumulative_average_n *
                                                                simulation_ui->simulation_delta_cumulative_average)) /
                                                              (simulation_ui->simulation_delta_cumulative_average_n + n_simulation_steps));
        simulation_ui->simulation_delta_cumulative_average_n += n_simulation_steps;
      }
    }
    else if (simulation_ui->mode == Mode::Editor)
    {
      if (files_loaded_state->cells_file_loaded)
      {
        do_cells_editor(cells_editor, state->universe, cell_initialisation_options, &loaded_rule->config.named_states, mouse_universe_pos, &mouse_click_consumed);
      }
    }

    //
    // Render
    //

    glViewport(0, 0, window_size.x, window_size.y);
    vec4 background_colour = get_state_colour(0);
    glClearColor(background_colour.x, background_colour.y, background_colour.z, background_colour.w);
    glClear(GL_COLOR_BUFFER_BIT);

    //
    // Cell instance drawing
    //

    if (files_loaded_state->cells_file_loaded && state->universe != 0)
    {
      upload_cell_instances(state->universe, simulate_options->border, cell_instancing);

      // Main view
      draw_cell_blocks(state->universe, cell_instancing, cell_drawing, general_vertex_buffer, view_panning->projection_matrix);

      // Minimap
      {
        if (vec2_eq(state->minimap_texture_size, {0, 0}))
        {
          state->minimap_texture_size = {300, 300};
          state->minimap_texture = create_minimap_texture(state->minimap_texture_size, state->minimap_framebuffer);
        }

        draw_minimap_texture(state->universe, cell_instancing, cell_drawing, general_vertex_buffer, state->minimap_framebuffer, state->minimap_texture, state->minimap_texture_size);
        opengl_print_errors();

        ImTextureID tex_id = (void *)(intptr_t)state->minimap_texture;
        ImGui::Image(tex_id, s32vec2_to_vec2(state->minimap_texture_size), ImVec2(0,0), ImVec2(1,1), ImColor(0xFF, 0xFF, 0xFF, 0xFF), ImColor(0xFF, 0xFF, 0xFF, 0x80));

        glViewport(0, 0, window_size.x, window_size.y);
      }
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

    BufferDrawingLocation uploaded_vertices = {};
    BufferDrawingLocation uploaded_indices = {};
    uploaded_indices.start_position = general_universe_ibo->elements_used;

    if (cells_editor->cell_highlighted)
    {
      vec4 highlighted_cell_colour = get_state_colour(cells_editor->highlighted_cell_state);
      highlighted_cell_colour = lighten_colour(highlighted_cell_colour);
      GeneralUnvierseVertex colour_template = {{{}, {}}, highlighted_cell_colour};

      u32 colour_template_position_offset = offsetof(GeneralUnvierseVertex, vertex);

      r32 cell_dim = 1.0 / state->universe->cell_block_dim;
      UniversePosition cell_size = {{}, {cell_dim, cell_dim}};

      upload_square(cells_editor->highlighted_cell, cell_size, colour_template, colour_template_position_offset,
                    general_universe_vbo, general_universe_ibo, &uploaded_vertices.n_elements, &uploaded_indices.n_elements);
    }

    // Get attribute locations
    init_general_universe_attributes(general_universe_vbo, state->general_universe_shader_program);

    glDrawElements(GL_TRIANGLES, uploaded_indices.n_elements, GL_UNSIGNED_SHORT, (void *)(intptr_t)(uploaded_indices.start_position*sizeof(GLushort)));

    opengl_print_errors();
    glBindVertexArray(0);

    //
    // Screen drawing
    //

    if (cell_selections_ui->making_selection || cell_selections_ui->selection_made)
    {
      BufferDrawingLocation vertices_position = {};
      BufferDrawingLocation colours_position = {};
      BufferDrawingLocation ibo_position = {};

      cell_selections_drawing_upload(cell_selections_ui, state->universe, view_panning->projection_matrix, aspect_ratio, &screen_shader->vertex_buffer, &screen_shader->colour_buffer, &screen_shader->index_buffer, &vertices_position, &colours_position, &ibo_position);

      glBindVertexArray(screen_shader->vao);
      glUseProgram(screen_shader->shader_program);
      initialise_screen_shader_attributes(screen_shader);

      glUniformMatrix4fv(screen_shader->aspect_ratio_uniform, 1, GL_TRUE, &aspect_ratio[0][0]);

      glBindBuffer(screen_shader->index_buffer.binding_target, screen_shader->index_buffer.id);
      glDrawElements(GL_TRIANGLES, ibo_position.n_elements, GL_UNSIGNED_SHORT, (void *)(intptr_t)(ibo_position.start_position*sizeof(GLushort)));

      glBindVertexArray(0);
      opengl_print_errors();

      screen_shader->vertex_buffer.elements_used -= vertices_position.n_elements;
      screen_shader->colour_buffer.elements_used -= colours_position.n_elements;
      screen_shader->index_buffer.elements_used -= ibo_position.n_elements;
    }

    //
    // Region placement drawing
    //

    if (cell_regions_ui->placing_region ||
        cell_regions_ui->placing_clipboard_region)
    {
      CellRegion *region;
      if (cell_regions_ui->placing_region)
      {
        region = Array::get(cell_regions->regions, cell_regions_ui->placing_region_index);
      }
      else
      {
        region = &cell_regions->clipboard_region;
      }

      Border region_border = {
        .type = BorderType::FIXED,
        .min_corner_block = region->start_block,
        .min_corner_cell = region->start_cell,
        .max_corner_block = region->end_block,
        .max_corner_cell = region->end_cell
      };
      upload_cell_instances(&region->cell_blocks, region_border, cell_instancing);

      vec2 start_offset = s32vec2_to_vec2(region->start_block);
      start_offset = vec2_add(start_offset, vec2_multiply(s32vec2_to_vec2(region->start_cell), -1.0 / region->cell_blocks.cell_block_dim));

      s32vec2 cell_dim = vec2_multiply(vec2_subtract(region->end_block, region->start_block), region->cell_blocks.cell_block_dim);
      cell_dim = vec2_add(cell_dim, region->end_cell);
      cell_dim = vec2_subtract(cell_dim, region->start_cell);
      vec2 midpoint_offset = vec2_multiply(s32vec2_to_vec2(vec2_multiply(cell_dim, 0.5)), 1.0/region->cell_blocks.cell_block_dim);

      vec2 mouse_cell_offset = mouse_universe_pos.cell_position;
      quantise_0to1_cell_position(mouse_cell_offset, region->cell_blocks.cell_block_dim);
      vec2 mouse_offset = vec2_add(s32vec2_to_vec2(mouse_universe_pos.cell_block_position), mouse_cell_offset);

      vec2 offset = {};
      offset = vec2_add(offset, mouse_offset);
      offset = vec2_add(offset, start_offset);
      offset = vec2_subtract(offset, midpoint_offset);

      vec3 offset_3 = {};
      offset_3.xy = offset;

      mat4x4 placing_region_projection_matrix;
      mat4x4Identity(placing_region_projection_matrix);
      mat4x4Translate(placing_region_projection_matrix, offset_3);

      mat4x4 placing_region_projection_matrix_aspect;
      mat4x4MultiplyMatrix(placing_region_projection_matrix_aspect, placing_region_projection_matrix, view_panning->projection_matrix);

      draw_cell_blocks(&region->cell_blocks, cell_instancing, cell_drawing, general_vertex_buffer, placing_region_projection_matrix_aspect);
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
    if (files_loaded_state->cells_file_loaded && state->universe != 0)
    {
      debug_universe_lines_ibo.n_elements += debug_cell_block_outline_drawing_upload(state->universe, general_universe_vbo, general_universe_ibo);
    }
#endif

#ifdef DEBUG_MOUSE_UNIVERSE_POSITION_DRAWING
    GeneralUnvierseVertex origin_vertex = {{0, 0}, {0, 0, 0, 1}};
    GeneralUnvierseVertex mouse_vertex = {mouse_universe_pos, {1, 0, 0, 1}};
    GLushort origin_index = opengl_buffer_new_element(general_universe_vbo, &origin_vertex);
    GLushort mouse_index = opengl_buffer_new_element(general_universe_vbo, &mouse_vertex);
    opengl_buffer_new_element(general_universe_ibo, &origin_index);
    opengl_buffer_new_element(general_universe_ibo, &mouse_index);

    debug_universe_lines_ibo.n_elements += 2;
#endif

#ifdef DEBUG_CELL_SELECTIONS_DRAWING
    if (files_loaded_state->cells_file_loaded && state->universe != 0)
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
    // Draw mouse position line
    vec2 vertices[] = {{0, 0}, state->screen_mouse_pos};
    vec4 colours[] = {{0, 0, 0, 1}, {0, 1, 0, 1}};
    u32 vertices_pos = opengl_buffer_add_elements(&screen_shader->vertex_buffer, array_count(vertices), vertices);
    u32 colours_pos = opengl_buffer_add_elements(&screen_shader->colour_buffer, array_count(colours), colours);

    glBindVertexArray(screen_shader->vao);
    glUseProgram(screen_shader->shader_program);
    initialise_screen_shader_attributes(screen_shader);

    mat4x4 id;
    mat4x4Identity(id);
    glUniformMatrix4fv(screen_shader->aspect_ratio_uniform, 1, GL_TRUE, &id[0][0]);

    glDrawArrays(GL_LINES, vertices_pos, array_count(vertices));

    screen_shader->vertex_buffer.elements_used -= array_count(vertices);
    screen_shader->colour_buffer.elements_used -= array_count(colours);

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
    ImGui::DestroyContext();
  }

  return result;
}


Export_VTable exports = { main_loop };


#ifdef __cplusplus
}
#endif


internal void
initialize_scene(Window *window, Game_Memory *memory)
{
    Game_State *game_state = (Game_State *)memory->permanent_storage;
    Memory_Arena *arena = &game_state->world_arena;
    Vertex_Buffers *vabo = &game_state->opengl_buffer;
    Gltf_Model *model = &game_state->character.model;
    Workbench_Settings *dbg_gui = &game_state->dbg_gui;
    initialize_arena(&game_state->world_arena, memory->permanent_storage_size - sizeof(Game_State),
                    (u8 *)memory->permanent_storage + sizeof(Game_State));

    vabo->vao = push_array(arena, 4, u32);
    vabo->count = push_array(arena, 4, u32);
    vabo->vbo = push_array(arena, 4, u32);
    vabo->ebo = push_array(arena, 4, u32);

    load_model(model, arena, "../data/vampire/vampire.glb");

    const char *vert_shader_path = "../shaders/light.vs";
    const char *frag_shader_path = "../shaders/light.fs";
    game_state->light_shader = shader(window->sdl_window, vert_shader_path, frag_shader_path);

    vert_shader_path = "../shaders/lightSource.vs";
    frag_shader_path = "../shaders/lightSource.fs";
    game_state->source_light_shader = shader(window->sdl_window, vert_shader_path, frag_shader_path);

    vert_shader_path = "../shaders/terrain.vs";
    frag_shader_path = "../shaders/terrain.fs";
    const char *tcs_shader_path = "../shaders/terrain.tcs";
    const char *tes_shader_path = "../shaders/terrain.tes";
    game_state->terrain_shader = shader(window->sdl_window, vert_shader_path, frag_shader_path, tcs_shader_path, tes_shader_path);

    vert_shader_path = "../shaders/anim_model.vs";
    frag_shader_path = "../shaders/light.fs";
    game_state->anim_shader = shader(window->sdl_window, vert_shader_path, frag_shader_path);

    u32 n_square_verts = array_count(cube_vertices);
    get_vao_without_ebo(vabo, cube_vertices, n_square_verts);
    game_state->light_source_cube = get_vao_without_ebo(vabo, cube_vertices, n_square_verts);

    game_state->terrain = generate_height_map(vabo, arena, "../data/iceland_heightmap.png", &game_state->terrain_texture, GL_MIRRORED_REPEAT);

    glEnable(GL_DEPTH_TEST);

    Camera *debug_camera = &game_state->debug_camera;

    glm::vec3 cam_pos   = glm::vec3(0.0f, 0.0f,  3.0f);
    glm::vec3 cam_up    = glm::vec3(0.0f, 1.0f,  0.0f);
    *debug_camera = init_camera(cam_pos, cam_up, -90.0f);
    debug_camera->fov = 60.0f;

    game_state->character.live_link = init_network(1234);

    game_state->mode = workbench_mode;

    if(model->n_skins > 0 && model->skins[0].joint_count > 0)
        dbg_gui->selected_joint = (i32)model->skins[0].joint_node_indices[0];

    memory->is_initialized = true;
}

internal i32
get_scene_viewport_width(Window *window, Game_State *game_state)
{
    i32 viewport_width = window->width;
    if(game_state->mode == workbench_mode)
    {
        ImGuiIO &io = ImGui::GetIO();
        f32 panel_width = io.DisplaySize.x > 700 ? 350.0f : io.DisplaySize.x * 0.48f;
        viewport_width -= (i32)(panel_width * io.DisplayFramebufferScale.x);
    }
    if(viewport_width < 1) viewport_width = 1;
    return viewport_width;
}

internal void
shutdown_scene(Game_State *game_state)
{
    shutdown_live_link(&game_state->character.live_link);

    Gltf_Model *model = &game_state->character.model;
    for(size_t i = 0; i < model->node_count; ++i)
    {
        Gltf_Node *node = &model->nodes[i];
        for(u32 j = 0; j < node->n_primitives; ++j)
        {
            Gltf_Primitive *primitive = &node->primitive[j];
            glDeleteVertexArrays(1, &primitive->vao);
            glDeleteBuffers(1, &primitive->vbo);
            glDeleteBuffers(1, &primitive->ebo);
        }
    }
    for(u32 i = 0; i < model->n_textures; ++i)
    {
        glDeleteTextures(1, &model->textures[i].id);
    }

    Vertex_Buffers *vabo = &game_state->opengl_buffer;
    glDeleteVertexArrays(vabo->n_vao, vabo->vao);
    glDeleteBuffers(vabo->n_vbo, vabo->vbo);
    glDeleteBuffers(vabo->n_ebo, vabo->ebo);
    glDeleteTextures(1, &game_state->terrain_texture.index);
    glDeleteProgram(game_state->light_shader);
    glDeleteProgram(game_state->source_light_shader);
    glDeleteProgram(game_state->terrain_shader);
    glDeleteProgram(game_state->anim_shader);
}

internal void
begin_scene_frame(Window *window, i32 viewport_width, Workbench_Settings *dbg_gui)
{
    glViewport(0, 0, viewport_width, window->height);
    glPolygonMode(GL_FRONT_AND_BACK, dbg_gui->wireframe ? GL_LINE : GL_FILL);
    glm::vec4 color = dbg_gui->clear_color;
    glClearColor(color.x * color.w, color.y * color.w, color.z * color.w, color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

internal void
update_scene_camera(Window *window, Game_State *game_state, Game_Input *input, i32 viewport_width)
{
    Workbench_Settings *dbg_gui = &game_state->dbg_gui;
    Game_Controller_Input *new_keyboard = get_controller(input, 0);
    Camera *debug_camera = &game_state->debug_camera;
    if(dbg_gui->reset_camera)
    {
        if(dbg_gui->show_terrain && !dbg_gui->show_character)
        {
            *debug_camera = init_camera(glm::vec3(0, 25, 40), glm::vec3(0, 1, 0), -90.0f, -35.0f);
        }
        else
        {
            *debug_camera = init_camera(glm::vec3(0, 0, 3), glm::vec3(0, 1, 0), -90.0f);
        }
        debug_camera->fov = 60.0f;
        first_mouse = true;
        dbg_gui->reset_camera = false;
    }

    if(new_keyboard->debug.ended_down && game_state->mode != workbench_mode)
    {
        game_state->mode = workbench_mode;
    }
    else if(new_keyboard->debug.ended_down && game_state->mode != free_camera_mode)
    {
        game_state->mode = free_camera_mode;
        first_mouse = true;
    }

    switch(game_state->mode)
    {
        case workbench_mode:
        {
            SDL_SetWindowRelativeMouseMode(window->sdl_window, 0);
        } break;

        case free_camera_mode:
        {
            SDL_SetWindowRelativeMouseMode(window->sdl_window, 1);
            set_camera_direction(window, input, debug_camera, &input->last_mouse_x, &input->last_mouse_y);
        } break;
    }

    if(!ImGui::GetIO().WantCaptureKeyboard)
        process_camera_inputs(debug_camera, new_keyboard, input->dt);
    debug_camera->projection = glm::perspective(glm::radians(debug_camera->fov),
        (f32)viewport_width / (f32)(window->height > 0 ? window->height : 1), 0.1f, 1000.0f);
}

internal void
update_scene_lights(Game_State *game_state, Game_Input *input, glm::vec3 active_light_positions[4])
{
    Workbench_Settings *dbg_gui = &game_state->dbg_gui;
    if(dbg_gui->animate_lights) game_state->light_time += input->dt;

    for(u32 x = 0; x < array_count(point_light_positions); ++x)
    {
        f32 radius = 1.5f;
        f32 speed = 2.0f;
        active_light_positions[x] = point_light_positions[x] +
                                  glm::vec3(sin(game_state->light_time * speed + x) * radius,
                                  0.0f,
                                  cos(game_state->light_time * speed + x) * radius);
    }
}

internal void
draw_light_markers(Game_State *game_state, glm::mat4 view, glm::vec3 active_light_positions[4])
{
    Camera *debug_camera = &game_state->debug_camera;
    Workbench_Settings *dbg_gui = &game_state->dbg_gui;
    Vertex_Buffers *vabo = &game_state->opengl_buffer;
    use_shader(game_state->source_light_shader);
    set_uniform(game_state->source_light_shader, "view", view);
    set_uniform(game_state->source_light_shader, "projection", debug_camera->projection);
    set_uniform(game_state->source_light_shader, "color", glm::vec3(1.0));

    glm::mat4 model = glm::mat4(1.0f);
    glBindVertexArray(vabo->vao[game_state->light_source_cube]);
    for(u32 x = 0; x < array_count(point_light_positions); ++x)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, active_light_positions[x]);
        model = glm::scale(model, glm::vec3(0.2f));

        set_uniform(game_state->source_light_shader, "model", model);

        glBindVertexArray(vabo->vao[game_state->light_source_cube]);
        if(dbg_gui->show_light_markers)
            glDrawArrays(GL_TRIANGLES, 0, vabo->count[game_state->light_source_cube]);
    }
}

internal void
draw_character(Game_State *game_state, glm::mat4 view, glm::vec3 active_light_positions[4],
              f32 time_value, glm::mat4 character_root)
{
    Camera *debug_camera = &game_state->debug_camera;
    Workbench_Settings *dbg_gui = &game_state->dbg_gui;
    Gltf_Model *model = &game_state->character.model;
    set_camera_uniforms(game_state->anim_shader, debug_camera, view);
    update_shader_lights(game_state->anim_shader, debug_camera, time_value, active_light_positions, true);

    set_uniform(game_state->anim_shader, "final_bones_matrices", model->final_bones_matrices, 100);
    i32 selected_joint = -1;
    if(dbg_gui->selected_joint >= 0 && (size_t)dbg_gui->selected_joint < model->node_count)
        selected_joint = find_skin_joint(model, &model->nodes[dbg_gui->selected_joint]);
    set_uniform(game_state->anim_shader, "selected_joint", selected_joint);
    set_uniformb(game_state->anim_shader, "show_weights", dbg_gui->show_weights && selected_joint >= 0);
    draw_model(model, game_state->anim_shader, character_root);
}

internal void
draw_character_skeleton(Window *window, i32 viewport_width, Game_State *game_state,
                      glm::mat4 view, glm::mat4 character_root)
{
    Camera *debug_camera = &game_state->debug_camera;
    Workbench_Settings *dbg_gui = &game_state->dbg_gui;
    Gltf_Model *model = &game_state->character.model;
    // Use the same mesh transform as draw_model and the skinning shader.
    glm::mat4 mesh_world = character_root;
    for(size_t i = 0; i < model->node_count; ++i)
    {
        if(model->nodes[i].n_primitives > 0)
        {
            mesh_world *= model->nodes[i].global_transform;
            break;
        }
    }
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 viewport_pos = ImGui::GetMainViewport()->Pos;
    glm::vec2 origin(viewport_pos.x, viewport_pos.y);
    glm::vec2 size((f32)viewport_width / io.DisplayFramebufferScale.x,
                (f32)window->height / io.DisplayFramebufferScale.y);
    draw_skeleton(model, dbg_gui, debug_camera->projection * view, mesh_world, origin, size);
}

internal void
draw_terrain(Game_State *game_state, glm::mat4 view, glm::vec3 active_light_positions[4], f32 time_value)
{
    Camera *debug_camera = &game_state->debug_camera;
    Workbench_Settings *dbg_gui = &game_state->dbg_gui;
    Vertex_Buffers *vabo = &game_state->opengl_buffer;
    set_camera_uniforms(game_state->terrain_shader, debug_camera, view);
    update_shader_lights(game_state->terrain_shader, debug_camera, time_value, active_light_positions, false);

    set_uniform(game_state->terrain_shader, "height_map", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, game_state->terrain_texture.index);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0, -10, 0));
    model = glm::scale(model, glm::vec3(0.2));

    set_uniform(game_state->terrain_shader, "model", model);

    set_uniform(game_state->terrain_shader, "scale", dbg_gui->scale);
    set_uniform(game_state->terrain_shader, "shift", dbg_gui->shift);

    set_uniform(game_state->terrain_shader, "shader_r",dbg_gui->shader_color.x*dbg_gui->shader_color.w);
    set_uniform(game_state->terrain_shader, "shader_g",dbg_gui->shader_color.y*dbg_gui->shader_color.w);
    set_uniform(game_state->terrain_shader, "shader_b",dbg_gui->shader_color.z*dbg_gui->shader_color.w);

    glm::vec2 u_texel_size = glm::vec2(game_state->terrain_texture.width, game_state->terrain_texture.height);
    set_uniform(game_state->terrain_shader, "u_texel_size", u_texel_size);

    glBindVertexArray(vabo->vao[game_state->terrain]);
    if(dbg_gui->show_terrain) glDrawArrays(GL_PATCHES, 0, 20*vabo->rez*vabo->rez);
}

void
game_update_and_render(Window *window, Game_Memory *memory, Game_Input *input)
{
    assert_slow((&input->controllers[0].terminator - &input->controllers[0].buttons[0]) ==
           (array_count(input->controllers[0].buttons)));
    assert_slow(sizeof(Game_State) <= memory->permanent_storage_size);

    if(!memory->is_initialized)
    {
        initialize_scene(window, memory);
    }

    Game_State *game_state = (Game_State *)memory->permanent_storage;
    Camera *debug_camera = &game_state->debug_camera;
    Workbench_Settings *dbg_gui = &game_state->dbg_gui;
    i32 viewport_width = get_scene_viewport_width(window, game_state);
    begin_scene_frame(window, viewport_width, dbg_gui);
    f32 time_value = get_seconds_elapsed(input->uptime, input->frequency);

    update_scene_camera(window, game_state, input, viewport_width);
    glm::mat4 view = get_debug_view_matrix(debug_camera);

    glm::vec3 active_light_positions[4];
    update_scene_lights(game_state, input, active_light_positions);
    draw_light_markers(game_state, view, active_light_positions);

    Character_State *character = &game_state->character;
    update_live_link(&character->live_link, &character->model);
    update_character(character, input->dt);
    if(dbg_gui->show_character)
    {
        glm::mat4 character_root = glm::translate(glm::mat4(1.0f), glm::vec3(0, -1.5f, 0));
        character_root = glm::rotate(character_root, glm::radians(-90.0f), glm::vec3(1, 0, 0));
        character_root = glm::scale(character_root, glm::vec3(100.02f));

        draw_character(game_state, view, active_light_positions, time_value, character_root);
        draw_character_skeleton(window, viewport_width, game_state, view, character_root);
    }

    draw_terrain(game_state, view, active_light_positions, time_value);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
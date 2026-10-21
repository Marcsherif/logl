internal i32
find_skin_joint(Gltf_Model *model, Gltf_Node *node)
{
    if(model->n_skins == 0) return -1;

    Gltf_Skin *skin = &model->skins[0];
    for(u32 i = 0; i < skin->joint_count; ++i)
    {
        if(&model->nodes[skin->joint_node_indices[i]] == node) return (i32)i;
    }
    return -1;
}

internal b32
has_skin_joint(Gltf_Model *model, Gltf_Node *node)
{
    if(find_skin_joint(model, node) >= 0) return true;

    for(size_t i = 0; i < node->child_count; ++i)
    {
        if(has_skin_joint(model, node->children[i])) return true;
    }
    return false;
}

internal b32
project_joint(glm::vec4 clip, glm::vec2 origin, glm::vec2 size, glm::vec2 *screen)
{
    // Do not divide joints behind the camera or outside the depth range.
    if(clip.w <= 0.0001f || clip.z < -clip.w || clip.z > clip.w) return false;

    glm::vec3 ndc = glm::vec3(clip) / clip.w;
    screen->x = origin.x + (ndc.x + 1.0f) * 0.5f * size.x;
    screen->y = origin.y + (1.0f - ndc.y) * 0.5f * size.y;
    return true;
}

internal void
draw_skeleton(Gltf_Model *model, Workbench_Settings *dbg_gui, glm::mat4 view_projection,
             glm::mat4 mesh_world, glm::vec2 origin, glm::vec2 size)
{
    if(model->n_skins == 0 || !dbg_gui->show_skeleton) return;

    ImDrawList *draw_list = ImGui::GetBackgroundDrawList();
    glm::vec2 corner = origin + size;
    draw_list->PushClipRect(ImVec2(origin.x, origin.y), ImVec2(corner.x, corner.y), true);

    Gltf_Skin *skin = &model->skins[0];
    glm::mat4 transform = view_projection * mesh_world;
    for(u32 i = 0; i < skin->joint_count; ++i)
    {
        u32 node_index = skin->joint_node_indices[i];
        Gltf_Node *node = &model->nodes[node_index];
        glm::vec2 point;
        if(!project_joint(transform * node->global_transform[3], origin, size, &point)) continue;

        b32 selected = dbg_gui->selected_joint == (i32)node_index;
        ImU32 color = IM_COL32(90, 205, 235, 230);
        if(selected) color = IM_COL32(255, 190, 60, 255);

        Gltf_Node *parent = node->parent;
        while(parent && find_skin_joint(model, parent) < 0)
        {
            parent = parent->parent;
        }
        if(parent)
        {
            glm::vec2 parent_point;
            if(project_joint(transform * parent->global_transform[3], origin, size, &parent_point))
                draw_list->AddLine(ImVec2(parent_point.x, parent_point.y), ImVec2(point.x, point.y), color, 2.0f);
        }

        f32 radius = 2.5f;
        if(selected) radius = 5.0f;
        draw_list->AddCircleFilled(ImVec2(point.x, point.y), radius, color);
        if(selected || dbg_gui->show_joint_names)
            draw_list->AddText(ImVec2(point.x + 7, point.y - 7), color, node->name);
    }
    draw_list->PopClipRect();
}

internal void
draw_joint_tree(Gltf_Model *model, Gltf_Node *node, Workbench_Settings *dbg_gui, u32 depth)
{
    if(!has_skin_joint(model, node)) return;

    i32 node_index = (i32)(node - model->nodes);
    i32 joint_index = find_skin_joint(model, node);
    b32 has_children = false;
    for(size_t i = 0; i < node->child_count; ++i)
    {
        if(has_skin_joint(model, node->children[i])) has_children = true;
    }

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if(!has_children) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    if(dbg_gui->selected_joint == node_index) flags |= ImGuiTreeNodeFlags_Selected;
    if(depth < 2) flags |= ImGuiTreeNodeFlags_DefaultOpen;

    const char *name = "Unnamed node";
    if(node->name[0]) name = node->name;

    ImGui::PushID(node_index);
    b32 open = ImGui::TreeNodeEx("joint", flags, "%s", name);
    if(ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen() && joint_index >= 0)
        dbg_gui->selected_joint = node_index;

    if(open && has_children)
    {
        for(size_t i = 0; i < node->child_count; ++i)
        {
            draw_joint_tree(model, node->children[i], dbg_gui, depth + 1);
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

internal void
draw_skeleton_inspector(Gltf_Model *model, Workbench_Settings *dbg_gui)
{
    if(model->n_skins == 0)
    {
        ImGui::TextUnformatted("This model has no skeleton.");
        return;
    }

    ImGui::Checkbox("Skeleton overlay", &dbg_gui->show_skeleton);
    ImGui::Checkbox("Joint names", &dbg_gui->show_joint_names);
    ImGui::Checkbox("Selected joint weights", &dbg_gui->show_weights);
    ImGui::TextWrapped("Select a joint below. The overlay shows joints through the mesh.");
    if(!dbg_gui->show_character)
        ImGui::TextWrapped("Enable Character in the Scene tab to see the overlay and weights.");

    ImGui::SeparatorText("Joint hierarchy");
    ImGui::Text("%u joints (first skin)", model->skins[0].joint_count);
    ImGui::BeginChild("Joint hierarchy", ImVec2(0, 220), ImGuiChildFlags_Borders);
    for(u32 i = 0; i < model->n_roots; ++i)
    {
        draw_joint_tree(model, &model->nodes[model->root_indices[i]], dbg_gui, 0);
    }
    ImGui::EndChild();

    if(dbg_gui->selected_joint < 0 || (size_t)dbg_gui->selected_joint >= model->node_count) return;

    Gltf_Node *node = &model->nodes[dbg_gui->selected_joint];
    ImGui::SeparatorText("Selected joint");
    ImGui::TextWrapped("%s", node->name);
    ImGui::Text("Node: %d   Skin joint: %d", dbg_gui->selected_joint, find_skin_joint(model, node));
    if(node->parent) ImGui::TextWrapped("Parent: %s", node->parent->name);

    ImGui::TextDisabled("Local transforms (read only)");
    ImGui::Text("Position: %.3f  %.3f  %.3f", node->translation.x, node->translation.y, node->translation.z);
    glm::vec3 angles = glm::degrees(glm::eulerAngles(node->rotation));
    ImGui::Text("Rotation: %.1f  %.1f  %.1f deg", angles.x, angles.y, angles.z);
    ImGui::Text("Scale:    %.3f  %.3f  %.3f", node->scale.x, node->scale.y, node->scale.z);
    if(node->use_baked_matrix)
        ImGui::TextWrapped("This node uses a baked local matrix instead of TRS.");

    if(dbg_gui->show_weights)
    {
        ImGui::SeparatorText("Weight colors");
        ImGui::TextColored(ImVec4(0.2f, 0.4f, 1, 1), "Blue: 0");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0, 0.9f, 0.4f, 1), "Green: 0.5");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1, 0.2f, 0.05f, 1), "Red: 1");
    }
    ImGui::TextWrapped("Pause or scrub in the Animation tab to inspect a pose.");
}

internal void
draw_workbench(Game_State *game_state)
{
    Workbench_Settings *dbg_gui = &game_state->dbg_gui;
    Character_State *character = &game_state->character;
    Gltf_Model *model = &character->model;
    if(game_state->mode != workbench_mode)
    {
        return;
    }

    ImGuiViewport *viewport = ImGui::GetMainViewport();
    f32 panel_width = 350.0f;
    if(viewport->WorkSize.x <= 700)
    {
        panel_width = viewport->WorkSize.x * 0.48f;
    }

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - panel_width, viewport->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(panel_width, viewport->WorkSize.y));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
    ImGui::Begin("LOGL Workbench", 0, window_flags);
    ImGui::TextColored(ImVec4(0.38f, 0.78f, 0.93f, 1), "RENDERING / ANIMATION");
    ImGui::TextWrapped("Explore one system at a time. F2 switches between the workbench and free camera.");
    ImGui::Separator();

    if(ImGui::BeginTabBar("Workbench tabs"))
    {
        if(ImGui::BeginTabItem("Scene"))
        {
            ImGui::TextUnformatted("Focus");
            if(ImGui::Button("Character##focus", ImVec2(-1, 0)))
            {
                dbg_gui->show_character = true;
                dbg_gui->show_terrain = false;
                dbg_gui->show_light_markers = false;
                dbg_gui->animate_lights = false;
                dbg_gui->reset_camera = true;
            }
            if(ImGui::Button("Terrain##focus", ImVec2(-1, 0)))
            {
                dbg_gui->show_character = false;
                dbg_gui->show_terrain = true;
                dbg_gui->show_light_markers = false;
                dbg_gui->animate_lights = false;
                dbg_gui->reset_camera = true;
            }
            if(ImGui::Button("Full scene", ImVec2(-1, 0)))
            {
                dbg_gui->show_character = true;
                dbg_gui->show_terrain = true;
                dbg_gui->show_light_markers = true;
                dbg_gui->animate_lights = true;
                dbg_gui->reset_camera = true;
            }
            ImGui::Spacing();
            ImGui::SeparatorText("Visibility");
            ImGui::Checkbox("Character", &dbg_gui->show_character);
            ImGui::Checkbox("Terrain", &dbg_gui->show_terrain);
            ImGui::SeparatorText("Camera");
            ImGui::SliderFloat("Field of view", &game_state->debug_camera.fov, 20, 110, "%.0f deg");
            if(ImGui::Button("Reset camera"))
            {
                dbg_gui->reset_camera = true;
            }

            ImGui::TextWrapped("F2: free camera / controls\nWASD: move   Shift: faster\nMouse: look   Enter: fullscreen\nEscape: quit");
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Animation"))
        {
            b32 live_pose = character->live_link_enabled && has_live_link_pose(&character->live_link, model);
            const char *pose_name = "Clip";
            if(character->default_pose)
            {
                pose_name = "Rest pose";
            }
            else if(live_pose)
            {
                pose_name = "Maya live";
            }

            ImGui::Text("Pose: %s", pose_name);
            ImGui::Checkbox("Rest pose", &character->default_pose);
            ImGui::BeginDisabled(character->default_pose || live_pose || model->n_animations == 0);
            if(model->n_animations > 0)
            {
                Gltf_Animation *clip = &model->animations[character->selected_clip];
                const char *clip_name = "Unnamed clip";
                if(clip->name[0]) clip_name = clip->name;
                if(ImGui::BeginCombo("Clip", clip_name))
                {
                    for(u32 i = 0; i < model->n_animations; ++i)
                    {
                        Gltf_Animation *animation = &model->animations[i];
                        const char *name = "Unnamed clip";
                        if(animation->name[0]) name = animation->name;

                        ImGui::PushID((i32)i);
                        if(ImGui::Selectable(name, character->selected_clip == (i32)i))
                        {
                            character->selected_clip = (i32)i;
                            character->animation_cursor = 0;
                            character->seek_animation = true;
                            character->reset_pose = true;
                        }
                        ImGui::PopID();
                    }
                    ImGui::EndCombo();
                }
                ImGui::Checkbox("Pause", &character->paused);
                ImGui::SliderFloat("Speed", &character->playback_rate, 0.1f, 2.0f, "%.2fx");
                f32 duration = model->animations[character->selected_clip].duration;
                if(ImGui::SliderFloat("Time", &character->animation_cursor, 0, duration, "%.2f s"))
                {
                    character->seek_animation = true;
                    character->paused = true;
                }
                if(ImGui::Button("Restart clip"))
                {
                    character->animation_cursor = 0;
                    character->seek_animation = true;
                }
                ImGui::Text("Duration: %.2f s", duration);
            }
            else
            {
                ImGui::TextUnformatted("No animation clips in this model.");
            }
            ImGui::EndDisabled();
            ImGui::SeparatorText("Maya live link");
            const char *connection = "Waiting on port 1234";
            if(character->live_link.connected) connection = "Connected";
            ImGui::Text("Connection: %s", connection);
            ImGui::Checkbox("Use live pose", &character->live_link_enabled);
            ImGui::TextWrapped("Live input takes over after a matching joint update. Turning it off returns to the clip. Rest pose overrides both sources.");
            ImGui::Text("Nodes: %zu   Skins: %u", model->node_count, model->n_skins);
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Skeleton"))
        {
            draw_skeleton_inspector(model, dbg_gui);
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Rendering"))
        {
            ImGui::Checkbox("Wireframe", &dbg_gui->wireframe);
            ImGui::Checkbox("Animate lights", &dbg_gui->animate_lights);
            ImGui::Checkbox("Light markers", &dbg_gui->show_light_markers);
            ImGui::ColorEdit3("Background", glm::value_ptr(dbg_gui->clear_color));
            ImGui::SeparatorText("Terrain");
            ImGui::BeginDisabled(!dbg_gui->show_terrain);
            ImGui::SliderFloat("Height scale", &dbg_gui->scale, 0, 200);
            ImGui::SliderFloat("Height offset", &dbg_gui->shift, 0, 100);
            ImGui::ColorEdit3("Terrain tint", glm::value_ptr(dbg_gui->shader_color));
            ImGui::EndDisabled();
            ImGui::TextWrapped("Enable Terrain in the Scene tab to inspect displacement. Wireframe reveals adaptive tessellation.");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::SeparatorText("Frame statistics");
    f32 fps = ImGui::GetIO().Framerate;
    f32 ms_per_frame = 0;
    if(fps > 0) ms_per_frame = 1000.0f / fps;
    ImGui::Text("%.2f ms / frame   %.0f FPS", ms_per_frame, fps);
    ImGui::TextDisabled("Includes VSync; not a GPU benchmark.");
    ImGui::End();
}

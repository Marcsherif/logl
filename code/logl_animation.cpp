internal void
restore_rest_pose(Gltf_Model *model)
{
    for(size_t i = 0; i < model->node_count; ++i)
    {
        Gltf_Node *node = &model->nodes[i];
        node->translation = node->rest_translation;
        node->rotation = node->rest_rotation;
        node->scale = node->rest_scale;
        node->local_transform = node->rest_local_transform;
    }
}

// Returns the index 'i' such that input[i] <= time < input[i+1]
internal u32
find_keyframe_index(Gltf_Sampler *sampler, f32 time)
{
    for(u32 i = 0; i < sampler->count - 1; ++i)
    {
        if(time < sampler->input[i + 1])
        {
            return i;
        }
    }
    // Fallback to the second-to-last frame
    return sampler->count - 2;
}

internal void
evaluate_animation(Gltf_Model *model, u32 anim_index, f32 sample_time)
{
    if(anim_index >= model->n_animations) return;
    Gltf_Animation *anim = &model->animations[anim_index];

    for(u32 i = 0; i < anim->channel_count; ++i)
    {
        Gltf_Channel *channel = &anim->channels[i];
        Gltf_Sampler *sampler = channel->sampler;
        if(!sampler || sampler->count == 0 || channel->node_index >= model->node_count) continue;
        Gltf_Node *target_node = &model->nodes[channel->node_index];

        // Find two keyframes
        u32 k0 = sample_time >= sampler->input[sampler->count - 1] ?
            sampler->count - 1 : sampler->count == 1 ? 0 : find_keyframe_index(sampler, sample_time);
        u32 k1 = k0 + 1 < sampler->count ? k0 + 1 : k0;

        // Calculate Lerp Factor
        f32 t0 = sampler->input[k0];
        f32 t1 = sampler->input[k1];
        f32 t = t1 > t0 ? glm::clamp((sample_time - t0) / (t1 - t0), 0.0f, 1.0f) : 0.0f;
        if(sampler->interpolation == step) t = 0.0f;

        // Apply based on the transformation type
        if(channel->path == translation)
        {
            glm::vec3 *values = (glm::vec3*)sampler->output;
            target_node->translation = glm::mix(values[k0], values[k1], t);
        }
        else if(channel->path == rotation)
        {
            glm::quat *values = (glm::quat*)sampler->output;
            target_node->rotation = glm::slerp(values[k0], values[k1], t);
        }
        else if(channel->path == scale)
        {
            glm::vec3 *values = (glm::vec3*)sampler->output;
            target_node->scale = glm::mix(values[k0], values[k1], t);
        }
    }
}

internal void
update_node_hierarchy(Gltf_Node *node, glm::mat4 parent_transform)
{
    if(!node->use_baked_matrix)
    {
        glm::mat4 t = glm::translate(glm::mat4(1.0f), node->translation);
        glm::mat4 r = glm::mat4_cast(node->rotation);
        glm::mat4 s = glm::scale(glm::mat4(1.0f), node->scale);
        node->local_transform = t * r * s;
    }

    node->global_transform = parent_transform * node->local_transform;

    for(u32 i = 0; i < node->child_count; ++i)
    {
        update_node_hierarchy(node->children[i], node->global_transform);
    }
}

internal void
update_model_matrices(Gltf_Model *model, glm::mat4 root_transform = glm::mat4(1.0f))
{
    for(u32 i = 0; i < model->n_roots; ++i)
    {
        u32 root_index = model->root_indices[i];
        update_node_hierarchy(&model->nodes[root_index], root_transform);
    }
}

internal void
compute_skinning(Gltf_Model *model)
{
    if(model->n_skins == 0) return;

    Gltf_Skin *skin = &model->skins[0];
    for(u32 i = 0; i < skin->joint_count; ++i)
    {
        Gltf_Node *bone_node = &model->nodes[skin->joint_node_indices[i]];
        model->final_bones_matrices[i] = bone_node->global_transform * skin->inverse_bind_matrices[i];
    }
}

internal void
update_character(Character_State *character, f32 dt)
{
    Gltf_Model *model = &character->model;
    if(character->reset_pose)
    {
        restore_rest_pose(model);
        character->reset_pose = false;
    }

    b32 live_pose = character->live_link_enabled && has_live_link_pose(&character->live_link, model);
    if(character->default_pose)
    {
        restore_rest_pose(model);
    }
    else if(!live_pose && model->n_animations > 0 &&
            character->selected_clip >= 0 && (u32)character->selected_clip < model->n_animations)
    {
        if(character->seek_animation) character->animation_time = character->animation_cursor;

        f32 advance = 0;
        if(!character->paused && !character->seek_animation)
            advance = dt * character->playback_rate;

        character->animation_time += advance;
        f32 duration = model->animations[character->selected_clip].duration;
        if(duration > 0 && advance != 0)
            character->animation_time = fmod(character->animation_time, duration);

        evaluate_animation(model, (u32)character->selected_clip, character->animation_time);
    }
    if(live_pose && !character->default_pose)
        smooth_live_link(&character->live_link, model, dt);

    character->seek_animation = false;
    character->animation_cursor = character->animation_time;
    update_model_matrices(model);
    compute_skinning(model);
}

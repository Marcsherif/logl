internal void
setup_primitive(Gltf_Primitive *primitive)
{
    u32 *vao = &primitive->vao;
    u32 *vbo = &primitive->vbo;
    u32 *ebo = &primitive->ebo;

    glGenVertexArrays(1, vao);
    glGenBuffers(1, vbo);
    glGenBuffers(1, ebo);

    glBindVertexArray(*vao);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);

    glBufferData(GL_ARRAY_BUFFER, primitive->n_verts*sizeof(Gltf_Vertex), &primitive->vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, primitive->n_indices*sizeof(u32), &primitive->indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Gltf_Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Gltf_Vertex), (void*)(offsetof(Gltf_Vertex, normal)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Gltf_Vertex), (void*)(offsetof(Gltf_Vertex, tex_coords)));

    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(Gltf_Vertex), (void*)(offsetof(Gltf_Vertex, joint_ids)));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Gltf_Vertex), (void*)(offsetof(Gltf_Vertex, weights)));

    glBindVertexArray(0);
}

internal void
process_texture(Gltf_Model *model, Memory_Arena *arena, Gltf_Primitive *prim,
               cgltf_texture_view *view, char *shader_type)
{
    if (!view->texture) return;

    cgltf_image *image = view->texture->image;
    u32 texture_id = 0;
    b32 found = 0;

    for(u32 i = 0; i < model->n_textures; ++i)
    {
        if(model->textures[i].tex_id == (void*)image)
        {
            prim->texture_index[prim->n_textures++] = model->textures[i].index;
            found = 1;
            break;
        }
    }

    if(!found)
    {
        if (image->uri) {
            char full_path[256];
            snprintf(full_path, sizeof(full_path), "%s%s", model->path, image->uri);
            texture_id = get_texture(full_path, GL_MIRRORED_REPEAT);
        } else {
            texture_id = load_embedded_texture(image);
        }

        model->textures[model->n_textures].id = texture_id;
        model->textures[model->n_textures].tex_id = (void*)image;
        model->textures[model->n_textures].type = shader_type;
        model->textures[model->n_textures].index = model->n_textures;

        prim->texture_index[prim->n_textures++] = model->n_textures;
        model->n_textures++;
    }
}

internal void
process_primitives(Gltf_Model *model, Memory_Arena *arena, cgltf_primitive *primitives,
                  Gltf_Primitive *prim)
{
    if(primitives->type != cgltf_primitive_type_triangles)
    {
        return;
    }

    cgltf_attribute *position = 0;
    cgltf_attribute *normal = 0;
    cgltf_attribute *tex_coords = 0;
    cgltf_attribute *joints = 0;
    cgltf_attribute *weights = 0;

    for(cgltf_size i = 0; i < primitives->attributes_count; ++i)
    {
        char *attribute_name = primitives->attributes[i].name;
        cgltf_attribute *gltf_attribute = &primitives->attributes[i];

        if(!strcmp(attribute_name, "POSITION"))   position   = gltf_attribute;
        if(!strcmp(attribute_name, "NORMAL"))     normal     = gltf_attribute;
        if(!strcmp(attribute_name, "TEXCOORD_0")) tex_coords  = gltf_attribute;
        if(!strcmp(attribute_name, "JOINTS_0"))   joints     = gltf_attribute;
        if(!strcmp(attribute_name, "WEIGHTS_0"))  weights    = gltf_attribute;
    }

    cgltf_size vertex_count = position->data->count;
    cgltf_size index_count = primitives->indices->count;

    Gltf_Vertex *vertices = push_array(arena, vertex_count, Gltf_Vertex);
    u32 *indices = push_array(arena, index_count, u32);

    for(cgltf_size i = 0; i < vertex_count; ++i)
    {
        Gltf_Vertex vertex = {};

        cgltf_accessor_read_float(position->data, i, glm::value_ptr(vertex.position), 3);
        cgltf_accessor_read_float(normal->data, i, glm::value_ptr(vertex.normal), 3);
        cgltf_accessor_read_float(tex_coords->data, i, glm::value_ptr(vertex.tex_coords), 2);

        if(joints)
            cgltf_accessor_read_uint(joints->data, i,
                                    (cgltf_uint*)glm::value_ptr(vertex.joint_ids), 4);
        if(weights)
            cgltf_accessor_read_float(weights->data, i,
                                      glm::value_ptr(vertex.weights), 4);

        vertices[i] = vertex;
    }

    for (cgltf_size i = 0; i < index_count; i++) {
        indices[i] = (u32)cgltf_accessor_read_index(primitives->indices, i);
    }

    // setup buffers
    prim->vertices = vertices;
    prim->n_verts = (u32)vertex_count;

    prim->indices = indices;
    prim->n_indices = (u32)index_count;

    setup_primitive(prim);

    model->n_verts += prim->n_verts;
    model->n_indices += prim->n_indices;

    // setup textures and materials
    cgltf_material *mat = primitives->material;
    prim->texture_index = push_array(arena, 8, u32);
    if(mat)
    {
        process_texture(model, arena, prim, &mat->pbr_metallic_roughness.base_color_texture, "diffuse");
        process_texture(model, arena, prim, &mat->specular.specular_texture, "specular");
    }
}

internal void
process_node(Gltf_Model *model, Memory_Arena *arena, cgltf_node *src, Gltf_Node *dst)
{
    if(src->name) strncpy(dst->name, src->name, sizeof(dst->name));
    dst->name[sizeof(dst->name) - 1] = '\0';

    dst->translation = glm::vec3(0.0f);
    dst->rotation    = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    dst->scale       = glm::vec3(1.0f);
    dst->local_transform = glm::mat4(1.0f);
    dst->use_baked_matrix = false;

    if(src->has_translation)
    {
        dst->translation = glm::make_vec3(src->translation);
        dst->rest_translation = dst->translation;
    }
    if(src->has_rotation)
    {
        cgltf_float *r = src->rotation;
        dst->rotation = glm::quat(r[3], r[0], r[1], r[2]);
        dst->rest_rotation = dst->rotation;
    }
    if(src->has_scale)
    {
        dst->scale = glm::make_vec3(src->scale);
        dst->rest_scale = dst->scale;
    }

    if(src->has_matrix)
    {
        dst->local_transform = glm::make_mat4(src->matrix);
        dst->rest_local_transform = dst->local_transform;
        dst->use_baked_matrix = true;
    }

    // Save defaults too: glTF may omit identity rotation/scale properties.
    dst->rest_translation = dst->translation;
    dst->rest_rotation = dst->rotation;
    dst->rest_scale = dst->scale;
    dst->rest_local_transform = dst->local_transform;

    if(src->mesh)
    {
        u32 prim_count = (u32)src->mesh->primitives_count;
        dst->n_primitives = prim_count;

        if(prim_count > 0)
        {
            dst->primitive = push_array(arena, prim_count, Gltf_Primitive);
            for(u32 i = 0; i < prim_count; ++i)
            {
                process_primitives(model, arena, &src->mesh->primitives[i], &dst->primitive[i]);
            }
        }
    }
}

internal void
get_name(const char *path, char *output)
{
    const char* last_slash = strrchr(path, '/');
    if (!last_slash) last_slash = strrchr(path, '\\');

    if (last_slash) {
        size_t length = last_slash - path + 1; // +1 to include the slash
        strncpy(output, path, length);
        output[length] = '\0';
    } else {
        output[0] = '\0'; // No folder found
    }
}

internal void
load_model(Gltf_Model *model, Memory_Arena *arena, const char *path)
{
    cgltf_options options = {};
    cgltf_data* data = NULL;

    if (cgltf_parse_file(&options, path, &data) != cgltf_result_success)
    {
        throw_error("Failed to load GLTF");
    }

    if (cgltf_load_buffers(&options, data, path) != cgltf_result_success)
    {
       throw_error("Failed to load GLTF Buffers");
    }

    model->textures = push_array(arena, TEXTURE_CAPACITY, Gltf_Texture);
    get_name(path, model->path);

    model->node_count = (u32)data->nodes_count;
    model->nodes = push_array(arena, model->node_count, Gltf_Node);

    for(u32 i = 0; i < model->node_count; ++i)
    {
        process_node(model, arena, &data->nodes[i], &model->nodes[i]);
    }

    for(u32 i = 0; i < model->node_count; ++i)
    {
        cgltf_node *src = &data->nodes[i];
        Gltf_Node *dst = &model->nodes[i];

        // Link Parent
        if(src->parent)
        {
            u32 parent_index = (u32)(src->parent - data->nodes);
            dst->parent = &model->nodes[parent_index];
        }

        // Link Children
        dst->child_count = src->children_count;
        if(dst->child_count > 0)
        {
            dst->children = push_array(arena, dst->child_count, Gltf_Node*);
            for(u32 c = 0; c < dst->child_count; ++c)
            {
                u32 child_index = (u32)(src->children[c] - data->nodes);
                dst->children[c] = &model->nodes[child_index];
            }
        }
    }

    model->n_roots = (u32)data->scene->nodes_count;
    model->root_indices = push_array(arena, model->n_roots, u32);
    for(u32 i = 0; i < model->n_roots; ++i)
    {
        model->root_indices[i] = (u32)(data->scene->nodes[i] - data->nodes);
    }

    model->n_skins = (u32)data->skins_count;
    model->skins = push_array(arena, model->n_skins, Gltf_Skin);

    for(u32 i = 0; i < model->n_skins; ++i)
    {
        cgltf_skin *src_skin = &data->skins[i];
        Gltf_Skin *dst_skin = &model->skins[i];

        if (src_skin->name) strncpy(dst_skin->name, src_skin->name, sizeof(dst_skin->name));

        dst_skin->joint_count = (u32)src_skin->joints_count;
        dst_skin->joint_node_indices = push_array(arena, dst_skin->joint_count, u32);
        dst_skin->inverse_bind_matrices = push_array(arena, dst_skin->joint_count, glm::mat4);

        for (u32 j = 0; j < dst_skin->joint_count; ++j)
        {
            dst_skin->joint_node_indices[j] = (u32)(src_skin->joints[j] - data->nodes);

            if(src_skin->inverse_bind_matrices)
                cgltf_accessor_read_float(src_skin->inverse_bind_matrices, j,
                        glm::value_ptr(dst_skin->inverse_bind_matrices[j]), 16);
        }
    }

    u32 animations_count = (u32)data->animations_count;
    model->n_animations = animations_count;
    model->animations = push_array(arena, model->n_animations, Gltf_Animation);
    for (u32 i = 0; i < animations_count; ++i)
    {
        cgltf_animation* src = &data->animations[i];
        Gltf_Animation* dst = &model->animations[i];

        if (src->name) strncpy(dst->name, src->name, sizeof(dst->name));

        dst->sampler_count = (u32)src->samplers_count;
        dst->samplers = push_array(arena, dst->sampler_count, Gltf_Sampler);
        for (u32 j = 0; j < dst->sampler_count; ++j)
        {
            cgltf_animation_sampler* src_sampler = &src->samplers[j];
            Gltf_Sampler* dst_sampler = &dst->samplers[j];

            dst_sampler->count = (u32)src_sampler->input->count;
            dst_sampler->interpolation = (Interpolation_Type)src_sampler->interpolation;

            dst_sampler->input = push_array(arena, dst_sampler->count, f32);
            for(u32 k = 0; k < dst_sampler->count; ++k) {
                cgltf_accessor_read_float(src_sampler->input, k, &dst_sampler->input[k], 1);
            }

            u32 num_components = (u32)cgltf_num_components(src_sampler->output->type);
            dst_sampler->output = push_array(arena, dst_sampler->count * num_components, f32);
            cgltf_accessor_unpack_floats(src_sampler->output, (f32*)dst_sampler->output, dst_sampler->count * num_components);

            // find the longest timestamp, to find the animation duration
            if(dst_sampler->count > 0)
            {
                f32 max_time = dst_sampler->input[dst_sampler->count - 1];
                if(max_time > dst->duration) dst->duration = max_time;
            }
        }

        dst->channel_count = (u32)src->channels_count;
        dst->channels = push_array(arena, dst->channel_count, Gltf_Channel);
        for (u32 j = 0; j < dst->channel_count; ++j)
        {
            cgltf_animation_channel* src_channel = &src->channels[j];
            Gltf_Channel* dst_channel = &dst->channels[j];

            dst_channel->node_index = (u32)(src_channel->target_node - data->nodes);

            u32 sampler_index = (u32)(src_channel->sampler - src->samplers);
            dst_channel->sampler = &dst->samplers[sampler_index];

            if(src_channel->target_path == cgltf_animation_path_type_translation)
                dst_channel->path = translation;
            if(src_channel->target_path == cgltf_animation_path_type_rotation)
                dst_channel->path = rotation;
            if(src_channel->target_path == cgltf_animation_path_type_scale)
                dst_channel->path = scale;
        }
    }

    for(u32 i = 0; i < model->node_count; ++i) {
        Gltf_Node *node = &model->nodes[i];

        // if(node->name) printf("\"%s\", ", node->name);

        node->target_translation = node->translation;
        node->target_rotation = node->rotation;
        node->has_live_target = false;

        model->bone_mapping[model->nodes[i].name] = i;
    }
    // printf("\n");

    cgltf_free(data);
}


internal void
draw_node(Gltf_Primitive *prim, glm::mat4 world, Gltf_Texture *textures, u32 n_textures, i32 shader)
{
    if(!prim || prim->vao == 0) return;

    glBindVertexArray(prim->vao);
    use_shader(shader);
    set_uniform(shader, "model", world);

    u32 *index_array = prim->texture_index;

    u32 diffuse_nr = 1;
    u32 specular_nr = 1;
    u32 normal_nr = 1;

    for(u32 i = 0; i < prim->n_textures; ++i)
    {
        u32 index = index_array[i];
        glActiveTexture(GL_TEXTURE0 + i);

        char *category = textures[index].type;
        char uniform_name[64] = {};
        u32 number = 0;

        if(!strcmp(category,"diffuse")) number = diffuse_nr++;
        else if(!strcmp(category, "specular")) number = specular_nr++;
        else if(!strcmp(category, "normal")) number = normal_nr++;

        snprintf(uniform_name, sizeof(uniform_name), "material.%s%d", category, number);

        i32 location = glGetUniformLocation(shader, uniform_name);
        if(location == -1) {
            log_message("WARNING: Shader [%d] has no uniform named [%s]\n", shader, uniform_name);
        }

        set_uniform(shader, uniform_name, (i32)i);
        glBindTexture(GL_TEXTURE_2D, textures[index].id);
    }

    glDrawElements(GL_TRIANGLES, prim->n_indices, GL_UNSIGNED_INT, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(0);
}

internal void
draw_model(Gltf_Model *model, i32 shader, glm::mat4 root = glm::mat4(1.0f))
{
    for(u32 i = 0; i < model->node_count; ++i)
    {
        Gltf_Node *node = &model->nodes[i];

        if(node->n_primitives > 0)
        {
            glm::mat4 model_matrix = root * node->global_transform;
            for(u32 p = 0; p < node->n_primitives; ++p)
            {
                draw_node(&node->primitive[p], model_matrix,
                         model->textures, model->n_textures, shader);
            }
        }
    }
}

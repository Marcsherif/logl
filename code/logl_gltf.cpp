internal void
SetupPrimitive(gltf_primitive *primitive)
{
    u32 *VAO = &primitive->VAO;
    u32 *VBO = &primitive->VBO;
    u32 *EBO = &primitive->EBO;

    glGenVertexArrays(1, VAO);
    glGenBuffers(1, VBO);
    glGenBuffers(1, EBO);

    glBindVertexArray(*VAO);
    glBindBuffer(GL_ARRAY_BUFFER, *VBO);

    glBufferData(GL_ARRAY_BUFFER, primitive->nVerts*sizeof(gltf_vertex), &primitive->vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, primitive->nIndices*sizeof(u32), &primitive->indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(gltf_vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(gltf_vertex), (void*)(offsetof(gltf_vertex, normal)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(gltf_vertex), (void*)(offsetof(gltf_vertex, texCoords)));

    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(gltf_vertex), (void*)(offsetof(gltf_vertex, jointIds)));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(gltf_vertex), (void*)(offsetof(gltf_vertex, weights)));

    glBindVertexArray(0);
}

internal void
ProcessTexture(gltf_model *model, memory_arena *arena, gltf_primitive *prim,
               cgltf_texture_view *view, char *shaderType)
{
    if (!view->texture) return;

    cgltf_image *image = view->texture->image;
    u32 textureID = 0;
    b32 found = 0;

    for(u32 i = 0; i < model->nTextures; ++i)
    {
        if(model->textures[i].tex_id == (void*)image)
        {
            prim->textureIndex[prim->nTextures++] = model->textures[i].index;
            found = 1;
            break;
        }
    }

    if(!found)
    {
        if (image->uri) {
            char fullPath[256];
            snprintf(fullPath, sizeof(fullPath), "%s%s", model->path, image->uri);
            textureID = GetTexture(fullPath, GL_MIRRORED_REPEAT);
        } else {
            textureID = LoadEmbeddedTexture(image);
        }

        model->textures[model->nTextures].id = textureID;
        model->textures[model->nTextures].tex_id = (void*)image;
        model->textures[model->nTextures].type = shaderType;
        model->textures[model->nTextures].index = model->nTextures;

        prim->textureIndex[prim->nTextures++] = model->nTextures;
        model->nTextures++;
    }
}

internal void
ProcessPrimitives(gltf_model *model, memory_arena *arena, cgltf_primitive *primitives,
                  gltf_primitive *prim)
{
    if(primitives->type != cgltf_primitive_type_triangles)
    {
        return;
    }

    cgltf_attribute *position = 0;
    cgltf_attribute *normal = 0;
    cgltf_attribute *texCoords = 0;
    cgltf_attribute *joints = 0;
    cgltf_attribute *weights = 0;

    for(cgltf_size i = 0; i < primitives->attributes_count; ++i)
    {
        char *attributeName = primitives->attributes[i].name;
        cgltf_attribute *gltfAttribute = &primitives->attributes[i];

        if(!strcmp(attributeName, "POSITION"))   position   = gltfAttribute;
        if(!strcmp(attributeName, "NORMAL"))     normal     = gltfAttribute;
        if(!strcmp(attributeName, "TEXCOORD_0")) texCoords  = gltfAttribute;
        if(!strcmp(attributeName, "JOINTS_0"))   joints     = gltfAttribute;
        if(!strcmp(attributeName, "WEIGHTS_0"))  weights    = gltfAttribute;
    }

    cgltf_size vertexCount = position->data->count;
    cgltf_size indexCount = primitives->indices->count;

    gltf_vertex *vertices = PushArray(arena, vertexCount, gltf_vertex);
    u32 *indices = PushArray(arena, indexCount, u32);

    for(cgltf_size i = 0; i < vertexCount; ++i)
    {
        gltf_vertex vertex = {};

        cgltf_accessor_read_float(position->data, i, glm::value_ptr(vertex.position), 3);
        cgltf_accessor_read_float(normal->data, i, glm::value_ptr(vertex.normal), 3);
        cgltf_accessor_read_float(texCoords->data, i, glm::value_ptr(vertex.texCoords), 2);

        if(joints)
            cgltf_accessor_read_uint(joints->data, i,
                                    (cgltf_uint*)glm::value_ptr(vertex.jointIds), 4);
        if(weights)
            cgltf_accessor_read_float(weights->data, i,
                                      glm::value_ptr(vertex.weights), 4);

        vertices[i] = vertex;
    }

    for (cgltf_size i = 0; i < indexCount; i++) {
        indices[i] = (u32)cgltf_accessor_read_index(primitives->indices, i);
    }

    // setup buffers
    prim->vertices = vertices;
    prim->nVerts = (u32)vertexCount;

    prim->indices = indices;
    prim->nIndices = (u32)indexCount;

    SetupPrimitive(prim);

    model->nVerts += prim->nVerts;
    model->nIndices += prim->nIndices;

    // setup textures and materials
    cgltf_material *mat = primitives->material;
    prim->textureIndex = PushArray(arena, 8, u32);
    if(mat)
    {
        ProcessTexture(model, arena, prim, &mat->pbr_metallic_roughness.base_color_texture, "diffuse");
        ProcessTexture(model, arena, prim, &mat->specular.specular_texture, "specular");
    }
}

internal void
ProcessNode(gltf_model *model, memory_arena *arena, cgltf_node *src, gltf_node *dst)
{
    if(src->name) strncpy(dst->name, src->name, sizeof(dst->name));

    dst->translation = glm::vec3(0.0f);
    dst->rotation    = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    dst->scale       = glm::vec3(1.0f);
    dst->localTransform = glm::mat4(1.0f);
    dst->useBakedMatrix = false;

    if(src->has_translation) dst->translation = glm::make_vec3(src->translation);
    if(src->has_rotation)
    {
        cgltf_float *r = src->rotation;
        dst->rotation = glm::quat(r[3], r[0], r[1], r[2]);
    }
    if(src->has_scale) dst->scale = glm::make_vec3(src->scale);

    if(src->has_matrix)
    {
        dst->localTransform = glm::make_mat4(src->matrix);
        dst->useBakedMatrix = true;
    }

    if(src->mesh)
    {
        u32 primCount = (u32)src->mesh->primitives_count;
        dst->nPrimitives = primCount;

        if(primCount > 0)
        {
            dst->primitive = PushArray(arena, primCount, gltf_primitive);
            for(u32 i = 0; i < primCount; ++i)
            {
                ProcessPrimitives(model, arena, &src->mesh->primitives[i], &dst->primitive[i]);
            }
        }
    }
}

internal void
GetName(char *path, char *output)
{
    const char* lastSlash = strrchr(path, '/');
    if (!lastSlash) lastSlash = strrchr(path, '\\');

    if (lastSlash) {
        size_t length = lastSlash - path + 1; // +1 to include the slash
        strncpy(output, path, length);
        output[length] = '\0';
    } else {
        output[0] = '\0'; // No folder found
    }
}

internal void
LoadModel(gltf_model *model, memory_arena *arena, char *path)
{
    cgltf_options options = {};
    cgltf_data* data = NULL;

    if (cgltf_parse_file(&options, path, &data) != cgltf_result_success)
    {
        ThrowError("Failed to load GLTF");
    }

    if (cgltf_load_buffers(&options, data, path) != cgltf_result_success)
    {
       ThrowError("Failed to load GLTF Buffers");
    }

    model->textures = PushArray(arena, TEXTURE_CAPACITY, gltf_texture);
    GetName(path, model->path);

    model->nodeCount = (u32)data->nodes_count;
    model->nodes = PushArray(arena, model->nodeCount, gltf_node);

    for(u32 i = 0; i < model->nodeCount; ++i)
    {
        ProcessNode(model, arena, &data->nodes[i], &model->nodes[i]);
    }

    for(u32 i = 0; i < model->nodeCount; ++i)
    {
        cgltf_node *src = &data->nodes[i];
        gltf_node *dst = &model->nodes[i];

        // Link Parent
        if(src->parent)
        {
            u32 parentIndex = (u32)(src->parent - data->nodes);
            dst->parent = &model->nodes[parentIndex];
        }

        // Link Children
        dst->childCount = src->children_count;
        if(dst->childCount > 0)
        {
            dst->children = PushArray(arena, dst->childCount, gltf_node*);
            for(u32 c = 0; c < dst->childCount; ++c)
            {
                u32 childIndex = (u32)(src->children[c] - data->nodes);
                dst->children[c] = &model->nodes[childIndex];
            }
        }
    }

    model->nRoots = (u32)data->scene->nodes_count;
    model->rootIndices = PushArray(arena, model->nRoots, u32);
    for(u32 i = 0; i < model->nRoots; ++i)
    {
        model->rootIndices[i] = (u32)(data->scene->nodes[i] - data->nodes);
    }

    model->nSkins = (u32)data->skins_count;
    model->skins = PushArray(arena, model->nSkins, gltf_skin);

    for(u32 i = 0; i < model->nSkins; ++i)
    {
        cgltf_skin *srcSkin = &data->skins[i];
        gltf_skin *dstSkin = &model->skins[i];

        if (srcSkin->name) strncpy(dstSkin->name, srcSkin->name, sizeof(dstSkin->name));

        dstSkin->jointCount = (u32)srcSkin->joints_count;
        dstSkin->jointNodeIndices = PushArray(arena, dstSkin->jointCount, u32);
        dstSkin->inverseBindMatrices = PushArray(arena, dstSkin->jointCount, glm::mat4);

        for (u32 j = 0; j < dstSkin->jointCount; ++j)
        {
            dstSkin->jointNodeIndices[j] = (u32)(srcSkin->joints[j] - data->nodes);

            if(srcSkin->inverse_bind_matrices)
                cgltf_accessor_read_float(srcSkin->inverse_bind_matrices, j,
                        glm::value_ptr(dstSkin->inverseBindMatrices[j]), 16);
        }
    }

    u32 animationsCount = (u32)data->animations_count;
    model->nAnimations = animationsCount;
    model->animations = PushArray(arena, model->nAnimations, gltf_animation);
    for (u32 i = 0; i < animationsCount; ++i)
    {
        cgltf_animation* src = &data->animations[i];
        gltf_animation* dst = &model->animations[i];

        if (src->name) strncpy(dst->name, src->name, sizeof(dst->name));

        dst->samplerCount = (u32)src->samplers_count;
        dst->samplers = PushArray(arena, dst->samplerCount, gltf_sampler);
        for (u32 j = 0; j < dst->samplerCount; ++j)
        {
            cgltf_animation_sampler* srcSampler = &src->samplers[j];
            gltf_sampler* dstSampler = &dst->samplers[j];

            dstSampler->count = (u32)srcSampler->input->count;
            dstSampler->interpolation = (interpolation_type)srcSampler->interpolation;

            dstSampler->input = PushArray(arena, dstSampler->count, f32);
            for(u32 k = 0; k < dstSampler->count; ++k) {
                cgltf_accessor_read_float(srcSampler->input, k, &dstSampler->input[k], 1);
            }

            u32 numComponents = (u32)cgltf_num_components(srcSampler->output->type);
            dstSampler->output = PushArray(arena, dstSampler->count * numComponents, f32);
            cgltf_accessor_unpack_floats(srcSampler->output, (f32*)dstSampler->output, dstSampler->count * numComponents);

            // find the longest timestamp, to find the animation duration
            if(dstSampler->count > 0)
            {
                f32 maxTime = dstSampler->input[dstSampler->count - 1];
                if(maxTime > dst->duration) dst->duration = maxTime;
            }
        }

        dst->channelCount = (u32)src->channels_count;
        dst->channels = PushArray(arena, dst->channelCount, gltf_channel);
        for (u32 j = 0; j < dst->channelCount; ++j)
        {
            cgltf_animation_channel* srcChannel = &src->channels[j];
            gltf_channel* dstChannel = &dst->channels[j];

            dstChannel->nodeIndex = (u32)(srcChannel->target_node - data->nodes);

            u32 samplerIndex = (u32)(srcChannel->sampler - src->samplers);
            dstChannel->sampler = &dst->samplers[samplerIndex];

            if(srcChannel->target_path == cgltf_animation_path_type_translation)
                dstChannel->path = translation;
            if(srcChannel->target_path == cgltf_animation_path_type_rotation)
                dstChannel->path = rotation;
            if(srcChannel->target_path == cgltf_animation_path_type_scale)
                dstChannel->path = scale;
        }
    }

    for(u32 i = 0; i < model->nodeCount; ++i) {
        gltf_node *node = &model->nodes[i];

        // if(node->name) printf("\"%s\", ", node->name);

        node->targetTranslation = node->translation;
        node->targetRotation = node->rotation;
    }
    // printf("\n");

    cgltf_free(data);
}


// Returns the index 'i' such that input[i] <= time < input[i+1]
internal u32
FindKeyframeIndex(gltf_sampler *sampler, f32 time)
{
    for (u32 i = 0; i < sampler->count - 1; ++i) {
        if (time < sampler->input[i + 1]) {
            return i;
        }
    }
    // Fallback to the second-to-last frame
    return sampler->count - 2;
}

internal void
UpdateAnimation(gltf_model *model, u32 animIndex, f32 time)
{
    gltf_animation *anim = &model->animations[animIndex];

    // advance time
    model->animationTime += time;
    if (anim->duration > 0) {
        model->animationTime = fmod(model->animationTime, anim->duration);
    }

    // pose step
    for (u32 i = 0; i < anim->channelCount; ++i) {
        gltf_channel *channel = &anim->channels[i];
        gltf_sampler *sampler = channel->sampler;
        gltf_node *targetNode = &model->nodes[channel->nodeIndex];

        // Find two keyframes
        u32 k0 = FindKeyframeIndex(sampler, model->animationTime);
        u32 k1 = k0 + 1;

        // Calculate Lerp Factor
        f32 t0 = sampler->input[k0];
        f32 t1 = sampler->input[k1];
        f32 t = (model->animationTime - t0) / (t1 - t0);

        // Apply based on the transformation type
        if (channel->path == translation) {
            glm::vec3 *values = (glm::vec3*)sampler->output;
            targetNode->translation = glm::mix(values[k0], values[k1], t);
        }
        else if (channel->path == rotation) {
            glm::quat *values = (glm::quat*)sampler->output;
            targetNode->rotation = glm::slerp(values[k0], values[k1], t);
        }
        else if (channel->path == scale) {
            glm::vec3 *values = (glm::vec3*)sampler->output;
            targetNode->scale = glm::mix(values[k0], values[k1], t);
        }
    }
}

internal void
ComputeSkinning(gltf_model *model)
{
    if (model->nSkins == 0) return;

    gltf_skin *skin = &model->skins[0];
    for (u32 i = 0; i < skin->jointCount; ++i) {
        gltf_node *boneNode = &model->nodes[skin->jointNodeIndices[i]];
        model->finalBonesMatrices[i] = boneNode->globalTransform * skin->inverseBindMatrices[i];
    }
}

internal void
DrawNode(gltf_primitive *prim, glm::mat4 world, gltf_texture *textures, u32 nTextures, i32 shader)
{
    if(!prim || prim->VAO == 0) return;

    glBindVertexArray(prim->VAO);
    UseShader(shader);
    SetUniform(shader, "model", world);

    u32 *indexArray = prim->textureIndex;

    u32 diffuseNr = 1;
    u32 specularNr = 1;
    u32 normalNr = 1;

    for(u32 i = 0; i < prim->nTextures; ++i)
    {
        u32 index = indexArray[i];
        glActiveTexture(GL_TEXTURE0 + i);

        char *category = textures[index].type;
        char uniformName[64] = {};
        u32 number = 0;

        if(!strcmp(category,"diffuse")) number = diffuseNr++;
        else if(!strcmp(category, "specular")) number = specularNr++;
        else if(!strcmp(category, "normal")) number = normalNr++;

        snprintf(uniformName, sizeof(uniformName), "material.%s%d", category, number);

        i32 location = glGetUniformLocation(shader, uniformName);
        if(location == -1) {
            Log("WARNING: Shader [%d] has no uniform named [%s]\n", shader, uniformName);
        }

        SetUniform(shader, uniformName, (i32)i);
        glBindTexture(GL_TEXTURE_2D, textures[index].id);
    }

    glDrawElements(GL_TRIANGLES, prim->nIndices, GL_UNSIGNED_INT, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(0);
}

internal void
UpdateNodeHierarchy(gltf_node *node, glm::mat4 parentTransform)
{
    if(!node->useBakedMatrix)
    {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), node->translation);
        glm::mat4 R = glm::mat4_cast(node->rotation);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), node->scale);
        node->localTransform = T * R * S;
    }

    node->globalTransform = parentTransform * node->localTransform;

    for(u32 i = 0; i < node->childCount; ++i)
    {
        UpdateNodeHierarchy(node->children[i], node->globalTransform);
    }
}

internal void
UpdateModelMatrices(gltf_model *model, glm::mat4 rootTransform = glm::mat4(1.0f))
{
    for(u32 i = 0; i < model->nRoots; ++i)
    {
        u32 rootIndex = model->rootIndices[i];
        UpdateNodeHierarchy(&model->nodes[rootIndex], rootTransform);
    }
}

internal void
DrawModel(gltf_model *model, i32 shader, glm::mat4 root = glm::mat4(1.0f))
{
    for(u32 i = 0; i < model->nodeCount; ++i)
    {
        gltf_node *node = &model->nodes[i];

        if(node->nPrimitives > 0)
        {
            glm::mat4 modelMatrix = root * node->globalTransform;
            for(u32 p = 0; p < node->nPrimitives; ++p)
            {
                DrawNode(&node->primitive[p], modelMatrix,
                         model->textures, model->nTextures, shader);
            }
        }
    }
}

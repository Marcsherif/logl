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

    glBindVertexArray(0);
}

internal void
DrawNode(gltf_node *node, gltf_texture *textures, u32 nTextures, i32 shader)
{
    gltf_primitive *prim = node->primitive;
    glBindVertexArray(prim->VAO);

    UseShader(shader);
    SetUniform(shader, "model", node->transform);

    u32 *indexArray = prim->textureIndex;

    u32 diffuseNr = 1;
    u32 specularNr = 1;
    for(u32 i = 0; i < prim->nTextures; ++i)
    {
        u32 index = indexArray[i];
        glActiveTexture(GL_TEXTURE0 + i);

        char *number = 0;
        char *name = textures[index].type;
        if(!strcmp(name,"diffuse"))
        {
            snprintf(number, sizeof(char), "%d", diffuseNr++);
        }
        else if(!strcmp(name, "specular"))
        {
            snprintf(number, sizeof(char), "%d", specularNr++);
        }

        char uniformName[64] = {};
        snprintf(uniformName, 64, "material.%s%s", name, number);

        SetUniform(shader, uniformName, (i32)textures[index].index);
        glBindTexture(GL_TEXTURE_2D, textures[index].id);
    }
    glActiveTexture(GL_TEXTURE0);

    glDrawElements(GL_TRIANGLES, prim->nIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void
CalculateNormals(gltf_vertex *vertices, u32 *indices, cgltf_size vertexCount, cgltf_size indexCount)
{
    for (cgltf_size i = 0; i < vertexCount; ++i)
    {
        vertices[i].normal = glm::vec3(0.0f);
    }

    for (cgltf_size i = 0; i < indexCount; i+=3)
    {
        glm::vec3 v0 = vertices[indices[i+0]].position;
        glm::vec3 v1 = vertices[indices[i+1]].position;
        glm::vec3 v2 = vertices[indices[i+2]].position;

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;

        glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

        vertices[indices[i+0]].normal += faceNormal;
        vertices[indices[i+1]].normal += faceNormal;
        vertices[indices[i+2]].normal += faceNormal;
    }

    // Normalize vertex normals
    for (cgltf_size i = 0; i < vertexCount; ++i)
    {
        vertices[i].normal = glm::normalize(vertices[i].normal);
    }
}

internal void
ProcessPrimitives(gltf_model *model, memory_arena *arena, cgltf_primitive *primitives, gltf_primitive *prim)
{
    if(primitives->type != cgltf_primitive_type_triangles)
    {
        return;
    }

    cgltf_attribute *position = 0;
    cgltf_attribute *normal = 0;
    cgltf_attribute *texCoords = 0;

    for(cgltf_size i = 0; i < primitives->attributes_count; ++i)
    {
        char *attributeName = primitives->attributes[i].name;
        cgltf_attribute *gltfAttribute = &primitives->attributes[i];
        if(!strcmp(attributeName, "POSITION"))
        {
            position = gltfAttribute;
        }

        if(!strcmp(attributeName, "NORMAL"))
        {
            normal = gltfAttribute;
        }

        if(!strcmp(attributeName, "TEXCOORD_0"))
        {
            texCoords = gltfAttribute;
        }
    }

    cgltf_size vertexCount = position->data->count;
    cgltf_size indexCount = primitives->indices->count;

    gltf_vertex *vertices = PushArray(arena, vertexCount, gltf_vertex);
    u32 *indices = PushArray(arena, indexCount, u32);

    for(cgltf_size i = 0; i < vertexCount; ++i)
    {
        gltf_vertex vertex = {};

        cgltf_accessor_read_float(position->data, i, glm::value_ptr(vertex.position), 4);
        cgltf_accessor_read_float(normal->data, i, glm::value_ptr(vertex.normal), 4);
        cgltf_accessor_read_float(texCoords->data, i, glm::value_ptr(vertex.texCoords), 4);

        vertices[i] = vertex;
    }

    for (cgltf_size i = 0; i < indexCount; i++) {
        indices[i] = (u32)cgltf_accessor_read_index(primitives->indices, i);
    }

    //CalculateNormals(vertices, indices, vertexCount, indexCount);
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
    prim->textureIndex = PushArray(arena, 2, u32);
    if(mat && mat->pbr_metallic_roughness.base_color_texture.texture)
    {
        gltf_texture diffuse;

        char *name = mat->pbr_metallic_roughness.base_color_texture.texture->image->name;
        diffuse.type = name;

        b32 skip = 0;
        if(!model->textures[0].type)
        {
            char fullPath[126];
            snprintf(fullPath, sizeof(fullPath), "../data/backpack/%s.jpg", name);
            diffuse.id = GetTexture(fullPath, GL_MIRRORED_REPEAT);

            model->textures[model->nTextures] = diffuse;
            model->textures[model->nTextures].index = model->nTextures;
            model->nTextures++;
        }

        for(u32 i = 0; i < model->nTextures; ++i)
        {
            if(!strcmp(model->textures[i].type, name))
            {
                prim->textureIndex[prim->nTextures++] = model->textures[i].index;
                skip = 1;
            }
        }

        if(!skip)
        {
            char fullPath[126];
            snprintf(fullPath, sizeof(fullPath), "../data/backpack/%s.jpg", name);
            diffuse.id = GetTexture(fullPath, GL_MIRRORED_REPEAT);

            model->textures[model->nTextures] = *PushStruct(arena, gltf_texture);
            model->textures[model->nTextures] = diffuse;
            model->textures[model->nTextures].index = model->nTextures;
            prim->textureIndex[prim->nTextures++] = model->nTextures;
            model->nTextures++;
        }
    }

    if(mat && mat->specular.specular_texture.texture)
    {
        gltf_texture specular;

        char *name = mat->specular.specular_texture.texture->image->name;
        specular.type = name;

        b32 skip = 0;
        if(!model->textures[0].type)
        {
            char fullPath[126];
            snprintf(fullPath, sizeof(fullPath), "../data/backpack/%s.jpg", name);
            specular.id = GetTexture(fullPath, GL_MIRRORED_REPEAT);

            model->textures[model->nTextures] = specular;
            model->textures[model->nTextures].index = model->nTextures;
            model->nTextures++;
        }

        for(u32 i = 0; i < model->nTextures; ++i)
        {
            if(!strcmp(model->textures[i].type, name))
            {
                prim->textureIndex[prim->nTextures++] = model->textures[i].index;
                skip = 1;
            }
        }

        if(!skip)
        {
            char fullPath[126];
            snprintf(fullPath, sizeof(fullPath), "../data/backpack/%s.jpg", name);
            specular.id = GetTexture(fullPath, GL_MIRRORED_REPEAT);

            model->textures[model->nTextures] = *PushStruct(arena, gltf_texture);
            model->textures[model->nTextures] = specular;
            model->textures[model->nTextures].index = model->nTextures;
            prim->textureIndex[prim->nTextures++] = model->nTextures;
            model->nTextures++;
        }
    }
}

internal void
ProcessNode(gltf_model *model, memory_arena *arena, cgltf_node *node, gltf_node *modelNode)
{
    glm::mat4 outMatrix(1.0f);
    glm::mat4 translation(1.0f);
    glm::mat4 rotation(1.0f);
    glm::mat4 scale(1.0f);

    if(node->has_matrix)
    {
        outMatrix *= glm::make_mat4(node->matrix);
    }
    else
    {
        if(node->has_translation)
        {
            cgltf_float *t = node->translation;
            translation = glm::translate(translation, glm::vec3(t[0], t[1], t[2]));
        }

        if(node->has_rotation)
        {
            cgltf_float *r = node->rotation;
            rotation = glm::mat4_cast(glm::quat(r[3], r[0], r[1], r[2]));

        }

        if(node->has_scale)
        {
            cgltf_float *s = node->scale;
            scale = glm::scale(scale, glm::vec3(s[0], s[1], s[2]));
        }

        outMatrix *= translation * rotation * scale;
    }

    modelNode->transform = outMatrix;

    if(node->mesh)
    {
        for(cgltf_size i = 0; i < node->mesh->primitives_count; ++i)
        {
            modelNode->primitive = PushStruct(arena, gltf_primitive);
            ProcessPrimitives(model, arena, &node->mesh->primitives[i], modelNode->primitive);
        }
    }

    gltf_node *currChild = model->root;
    for(cgltf_size i = 0; i < node->children_count; ++i)
    {
        ProcessNode(model, arena, node->children[i], currChild);
        currChild = PushStruct(arena, gltf_node);
        currChild = currChild->child;
        currChild->parent = currChild;
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

    cgltf_scene *scene = data->scene;

    model->textures = PushStruct(arena, gltf_texture);
    model->root = PushStruct(arena, gltf_node);
    model->root->parent = 0;

    gltf_node *currChild = model->root;
    for(cgltf_size i = 0; i < scene->nodes_count; ++i)
    {
        ProcessNode(model, arena, scene->nodes[i], currChild);
        currChild->child = PushStruct(arena, gltf_node);
        currChild = currChild->child;
        currChild->parent = currChild;
    }

    cgltf_free(data);
}

internal void
DrawModel(gltf_model *model, i32 shader)
{
    gltf_node *node = model->root->child;
    while(node->primitive)
    {
        DrawNode(node, model->textures, model->nTextures, shader);
        node = node->child;
    }
}

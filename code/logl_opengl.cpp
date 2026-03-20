internal void
FramebufferSizeCallback(i32 width, i32 height)
{
    glViewport(0, 0, width, height);
}

u32 GetVAOwithoutEBO(VABO *vabo, f32 *vertices, u32 nVerts)
{
    u32 vaoIndex = vabo->nVAO;
    vabo->count[vabo->nVAO] = nVerts;

    glGenVertexArrays(1, &vabo->VAO[vabo->nVAO]);
    glGenBuffers(1, &vabo->VBO[vabo->nVBO]);

    glBindBuffer(GL_ARRAY_BUFFER, vabo->VBO[vabo->nVBO++]);
    glBufferData(GL_ARRAY_BUFFER, nVerts*sizeof(*vertices + 0), vertices, GL_STATIC_DRAW);

    glBindVertexArray(vabo->VAO[vabo->nVAO++]);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    return vaoIndex;
}

u32 GetVAOwithEBO(VABO *vabo, f32 *vertices, u32 nVerts, u32 *indices, u32 nIndices)
{
    u32 vaoIndex = vabo->nVAO;
    vabo->count[vaoIndex] = nIndices;

    glGenVertexArrays(1, &vabo->VAO[vabo->nVAO]);

    glBindVertexArray(vabo->VAO[vabo->nVAO++]);

    glGenBuffers(1, &vabo->VBO[vabo->nVBO]);
    glBindBuffer(GL_ARRAY_BUFFER, vabo->VBO[vabo->nVBO++]);
    glBufferData(GL_ARRAY_BUFFER, nVerts*sizeof(*vertices + 0), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &vabo->EBO[vabo->nEBO]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vabo->EBO[vabo->nEBO++]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, nIndices*sizeof(*indices + 0), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    return vaoIndex;
}

internal u32
UploadToGPU(unsigned char* pixels, int width, int height, int channels)
{
    u32 textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    return textureID;
}

internal u32
LoadEmbeddedTexture(cgltf_image* image)
{
    if (!image->buffer_view) return 0;

    unsigned char* data_ptr = (unsigned char*)image->buffer_view->buffer->data;
    unsigned char* offset_ptr = data_ptr + image->buffer_view->offset;
    size_t size = image->buffer_view->size;

    int width, height, channels;
    unsigned char* pixels = stbi_load_from_memory(offset_ptr, (int)size, &width, &height, &channels, 4);

    u32 textureID = 0;
    if (pixels) {
        textureID = UploadToGPU(pixels, width, height, 4);
        stbi_image_free(pixels);
    } else {
        printf("Failed to load embedded texture from GLB buffer.\n");
    }

    return textureID;
}

u32 GetTexture(char *texturePath, u32 wrappingMethod)
{
    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    i32 imageWidth, imageHeight, nrChannels;
    u8 *data = stbi_load(texturePath, &imageWidth, &imageHeight, &nrChannels, 0);
    stbi_set_flip_vertically_on_load(true);
    if(data)
    {
        GLenum format = 0;
        if(nrChannels == 1)
            format = GL_RED;
        else if(nrChannels == 3)
            format = GL_RGB;
        else if(nrChannels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrappingMethod);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrappingMethod);

        //f32 bColor[] = {1.0f, 0.0f, 0.0f, 1.0f};
        //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, bColor);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    else
    {
        Log("[Texture] Failed to load texture %s", texturePath);
        stbi_image_free(data);
    }

    return texture;
}

u32 GenerateHeightMap(VABO *vabo, memory_arena *arena, char *heightMapPath, Texture *texture, u32 wrappingMethod)
{
    u32 vaoIndex = 0;

    glGenTextures(1, &texture->index);
    glBindTexture(GL_TEXTURE_2D, texture->index);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrappingMethod);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrappingMethod);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int nrChannels;
    u8 *data = stbi_load(heightMapPath, &texture->width, &texture->height, &nrChannels, 0);
    stbi_set_flip_vertically_on_load(true);
    int imageWidth = texture->width;
    int imageHeight = texture->height;
    if(data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        //glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);

        glPatchParameteri(GL_PATCH_VERTICES, 4);

        vabo->rez = 20;
        u32 nVerts = 20*vabo->rez*vabo->rez;

        GLfloat *vertices = PushArray(arena, nVerts, GLfloat);
        GLfloat *verticesPtr = vertices;

        for(u32 i = 0; i <= vabo->rez-1; ++i)
        {
            for(u32 j = 0; j <= vabo->rez-1; ++j)
            {
                *verticesPtr++ = -imageWidth/2.0f + imageWidth*i/(float)vabo->rez; // v.x
                *verticesPtr++ = 0.0f; // v.y
                *verticesPtr++ = -imageHeight/2.0f + imageHeight*j/(float)vabo->rez; // v.z
                *verticesPtr++ = i / (float)vabo->rez; // u
                *verticesPtr++ = j / (float)vabo->rez; // v

                *verticesPtr++ = -imageWidth/2.0f + imageWidth*(i+1)/(float)vabo->rez; // v.x
                *verticesPtr++ = 0.0f; // v.y
                *verticesPtr++ = -imageHeight/2.0f + imageHeight*j/(float)vabo->rez; // v.z
                *verticesPtr++ = (i+1) / (float)vabo->rez; // u
                *verticesPtr++ = j / (float)vabo->rez; // v

                *verticesPtr++ = -imageWidth/2.0f + imageWidth*i/(float)vabo->rez; // v.x
                *verticesPtr++ = 0.0f; // v.y
                *verticesPtr++ = -imageHeight/2.0f + imageHeight*(j+1)/(float)vabo->rez; // v.z
                *verticesPtr++ = i / (float)vabo->rez; // u
                *verticesPtr++ = (j+1) / (float)vabo->rez; // v

                *verticesPtr++ = -imageWidth/2.0f + imageWidth*(i+1)/(float)vabo->rez; // v.x
                *verticesPtr++ = 0.0f; // v.y
                *verticesPtr++ = -imageHeight/2.0f + imageHeight*(j+1)/(float)vabo->rez; // v.z
                *verticesPtr++ = (i+1) / (float)vabo->rez; // u
                *verticesPtr++ = (j+1) / (float)vabo->rez; // v
            }
        }

        vaoIndex = vabo->nVAO;
        vabo->count[vabo->nVAO] = nVerts;

        glGenVertexArrays(1, &vabo->VAO[vabo->nVAO]);
        glGenBuffers(1, &vabo->VBO[vabo->nVBO]);

        glBindBuffer(GL_ARRAY_BUFFER, vabo->VBO[vabo->nVBO++]);
        glBufferData(GL_ARRAY_BUFFER, nVerts*sizeof(*vertices + 0), vertices, GL_STATIC_DRAW);

        glBindVertexArray(vabo->VAO[vabo->nVAO++]);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)(3*sizeof(GL_FLOAT)));
        glEnableVertexAttribArray(1);
    }
    else
    {
        Log("[Texture] Failed to load heightMap %s", heightMapPath);
        stbi_image_free(data);
    }


    return vaoIndex;
}

u32 GenerateHeightMapCPU(VABO *vabo, memory_arena *arena, char *heightMapPath)
{
    u32 vaoIndex = 0;
    i32 imageWidth, imageHeight, nrChannels;
    stbi_us *data = stbi_load_16(heightMapPath, &imageWidth, &imageHeight, &nrChannels, 0);
    if(data)
    {
        i32 nVerts = imageWidth * imageHeight * 3;
        GLfloat *vertices = PushArray(arena, nVerts, GLfloat);
        GLfloat *verticesPtr = vertices;

        f32 yScale = 100.0f / 65535.0f;
        f32 yShift = 0.0025f;
        for(i32 y = 0; y < imageHeight; ++y)
        {
            for(i32 x = 0; x < imageWidth; ++x)
            {
                stbi_us texel = *(data + ((x + (y * imageWidth)) * nrChannels));

                *verticesPtr++ = -imageWidth / 2.0f + x;
                *verticesPtr++ = texel * yScale - yShift;
                *verticesPtr++ = -imageHeight / 2.0f + y;
            }
        }
        stbi_image_free(data);

        i32 nIndices = (imageWidth * 2) * (imageHeight-1);
        u32 *indices = PushArray(arena, nIndices, u32);
        u32 *indicesPtr = indices;
        for(int y = 0; y < imageHeight-1; y++)
        {
            for(int x = 0; x < imageWidth; x++)
            {
                for(int k = 0; k < 2; k++)
                {
                    *indicesPtr++ = (x + imageWidth * (y + k));
                }
            }
        }

        vabo->NUM_STRIPS = imageHeight - 1;
        vabo->NUM_VERTS_PER_STRIP = imageWidth * 2;

        vaoIndex = vabo->nVAO;
        vabo->count[vaoIndex] = nIndices;

        glGenVertexArrays(1, &vabo->VAO[vabo->nVAO]);
        glBindVertexArray(vabo->VAO[vabo->nVAO++]);

        glGenBuffers(1, &vabo->VBO[vabo->nVBO]);
        glBindBuffer(GL_ARRAY_BUFFER, vabo->VBO[vabo->nVBO++]);
        glBufferData(GL_ARRAY_BUFFER, nVerts*sizeof(vertices[0]), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GL_FLOAT), (void*)0);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &vabo->EBO[vabo->nEBO]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vabo->EBO[vabo->nEBO++]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, nIndices*sizeof(indices[0]), indices, GL_STATIC_DRAW);
    }
    else
    {
        Log("[Texture] Failed to load heightMap %s", heightMapPath);
        stbi_image_free(data);
    }

    return vaoIndex;
}


internal u32
shader(SDL_Window *sdl_window, const char *vertex_path, const char *fragment_path,
       const char *tcs_path = 0, const char *tes_path = 0)
{
    // TODO: Load Files and error check, and do hot realoding, and do a hash
    SDL_IOStream *vert_file = SDL_IOFromFile(vertex_path, "rb");
    SDL_IOStream *frag_file = SDL_IOFromFile(fragment_path, "rb");

    const i64 vert_file_size = SDL_GetIOSize(vert_file);
    const i64 frag_file_size = SDL_GetIOSize(frag_file);

    char *vert_ptr = (char *)VirtualAlloc(0, vert_file_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    char *frag_ptr = (char *)VirtualAlloc(0, frag_file_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    size_t vert_file_read = SDL_ReadIO(vert_file, vert_ptr, vert_file_size);
    size_t frag_file_read = SDL_ReadIO(frag_file, frag_ptr, frag_file_size);

    GLuint tcs_shader = 0;
    GLuint tes_shader = 0;
    char *tcs_ptr = 0;
    char *tes_ptr = 0;
    if((tcs_path && tes_path) != 0 )
    {
        SDL_IOStream *tcs_file = SDL_IOFromFile(tcs_path, "rb");
        SDL_IOStream *tes_file = SDL_IOFromFile(tes_path, "rb");

        const i64 tcs_file_size = SDL_GetIOSize(tcs_file);
        const i64 tes_file_size = SDL_GetIOSize(tes_file);

        tcs_ptr = (char *)VirtualAlloc(0, tcs_file_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        tes_ptr = (char *)VirtualAlloc(0, tes_file_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

        size_t tcs_file_read = SDL_ReadIO(tcs_file, tcs_ptr, tcs_file_size);
        size_t tes_file_read = SDL_ReadIO(tes_file, tes_ptr, tes_file_size);

        SDL_CloseIO(tcs_file);
        SDL_CloseIO(tes_file);

        tcs_shader = glCreateShader(GL_TESS_CONTROL_SHADER);
        glShaderSource(tcs_shader, 1, &tcs_ptr, NULL);
        glCompileShader(tcs_shader);

        tes_shader = glCreateShader(GL_TESS_EVALUATION_SHADER);
        glShaderSource(tes_shader, 1, &tes_ptr, NULL);
        glCompileShader(tes_shader);
    }

    SDL_CloseIO(vert_file);
    SDL_CloseIO(frag_file);

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vert_ptr, NULL);
    glCompileShader(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &frag_ptr, NULL);
    glCompileShader(fragment_shader);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    if((tcs_path && tes_path) != 0 )
    {
        glAttachShader(shader_program, tcs_shader);
        glAttachShader(shader_program, tes_shader);
    }
    glLinkProgram(shader_program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    GLint linked = false;
    GLint compiled = false;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &linked);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
    if((tcs_path && tes_path) != 0 )
    {
        glGetShaderiv(tcs_shader, GL_COMPILE_STATUS, &compiled);
        glGetShaderiv(tes_shader, GL_COMPILE_STATUS, &compiled);
    }
    if(!(linked && compiled))
    {
        char vertex_errors[4096];
        char fragment_errors[4096];
        char program_errors[4096];
        glGetShaderInfoLog(vertex_shader, sizeof(vertex_errors), NULL, vertex_errors);
        glGetShaderInfoLog(fragment_shader, sizeof(fragment_errors), NULL, fragment_errors);
        glGetProgramInfoLog(shader_program, sizeof(program_errors), NULL, program_errors);

        log_message(vertex_errors);
        log_message(fragment_errors);
        log_message(program_errors);
        if((tcs_path && tes_path) != 0 )
        {
            char tcs_errors[4096];
            char tes_errors[4096];
            glGetShaderInfoLog(tcs_shader, sizeof(tcs_errors), NULL, tcs_errors);
            glGetShaderInfoLog(tes_shader, sizeof(tes_errors), NULL, tes_errors);
            log_message(tcs_errors);
            log_message(tes_errors);
        }
        err("shader error");
    }
    log_message("[Vertex Shader] %s Initialized", vertex_path);
    log_message("[Fragment Shader] %s Initialized", fragment_path);
    if((tcs_path && tes_path) != 0 )
    {
        log_message("[TCS Shader] %s Initialized", tcs_path);
        log_message("[TES Shader] %s Initialized", tes_path);
        VirtualFree(tcs_ptr, 0, MEM_RELEASE);
        VirtualFree(tes_ptr, 0, MEM_RELEASE);
    }
    log_message("[Shader] linking successful");

    VirtualFree(vert_ptr, 0, MEM_RELEASE);
    VirtualFree(frag_ptr, 0, MEM_RELEASE);

    return shader_program;
}

internal void
use_shader(u32 id)
{
    glUseProgram(id);
}

internal void
set_uniformb(u32 id, const char *name, b32 value)
{
    glUniform1i(glGetUniformLocation(id, name), (int)value);
}

internal void
set_uniform(u32 id, const char *name, i32 value)
{
    glUniform1i(glGetUniformLocation(id, name), value);
}

internal void
set_uniform(u32 id, const char *name, f32 value)
{
    glUniform1f(glGetUniformLocation(id, name), value);
}

internal void
set_uniform(u32 id, const char *name, glm::vec2 value)
{
    glUniform2f(glGetUniformLocation(id, name), value.x, value.y);
}

internal void
set_uniform(u32 id, const char *name, glm::vec3 value)
{
    glUniform3f(glGetUniformLocation(id, name), value.x, value.y, value.z);
}

internal void
set_uniform(u32 id, const char *name, f32 x, f32 y)
{
    glUniform2f(glGetUniformLocation(id, name), x, y);
}

internal void
set_uniform(u32 id, const char *name, f32 x, f32 y, f32 z)
{
    glUniform3f(glGetUniformLocation(id, name), x, y, z);
}

internal void
set_uniform(u32 id, const char *name, glm::mat4 value)
{
    glUniformMatrix4fv(glGetUniformLocation(id, name), 1, GL_FALSE,
                       glm::value_ptr(value));
}

internal void
set_uniform(u32 id, const char *name, glm::mat4 *values, u32 count)
{
    glUniformMatrix4fv(glGetUniformLocation(id, name), count, GL_FALSE,
                       (f32*)values);
}

internal void
set_camera_uniforms(u32 shader, Camera *cam, glm::mat4 view_matrix)
{
    use_shader(shader);
    set_uniform(shader, "view_pos", cam->position);
    set_uniform(shader, "projection", cam->projection);
    set_uniform(shader, "view", view_matrix);
}

internal void
update_shader_lights(u32 shader, Camera *debug_camera, f32 total_time,
                   glm::vec3 active_pos[4], b32 point_lights)
{
    use_shader(shader);

    glm::vec3 light_color;
    light_color.x = 1.0;//0.4f;
    light_color.y = 1.0;//0.7f;
    light_color.z = 1.0;//0.1f;

    // spot_light
    set_uniform(shader, "spot_light.ambient", 0.0f, 0.0f, 0.0f);
    set_uniform(shader, "spot_light.diffuse", 1.0f, 1.0f, 1.0f);
    set_uniform(shader, "spot_light.specular", 1.0f, 1.0f, 1.0f);
    set_uniform(shader, "spot_light.position", debug_camera->position);
    set_uniform(shader, "spot_light.direction", debug_camera->front);
    set_uniform(shader, "spot_light.constant", 1.0f);
    set_uniform(shader, "spot_light.linear", 0.09f);
    set_uniform(shader, "spot_light.quadratic", 0.032f);
    set_uniform(shader, "spot_light.cut_off", (f32)glm::cos(glm::radians(12.5)));
    set_uniform(shader, "spot_light.outer_cut_off", (f32)glm::cos(glm::radians(15.0)));

    // directional light
    set_uniform(shader, "dir_light.direction", -0.2f, -1.0f, -0.3f);
    set_uniform(shader, "dir_light.ambient", 0.05f, 0.05f, 0.05f);
    set_uniform(shader, "dir_light.diffuse", 0.4f, 0.4f, 0.4f);
    set_uniform(shader, "dir_light.specular", 0.5f, 0.5f, 0.5f);

    if (point_lights)
    {
        // point light 1-4
        char position[30];
        char ambient[30];
        char diffuse[30];
        char specular[30];
        char constant[30];
        char linear[30];
        char quadratic[30];

        for(u32 x = 0; x < 4; ++x)
        {
            glm::vec3 p = active_pos[x];

            snprintf(position, sizeof(position), "point_lights[%d].position", x);
            snprintf(ambient,  sizeof(ambient),  "point_lights[%d].ambient",  x);
            snprintf(diffuse,  sizeof(diffuse),  "point_lights[%d].diffuse",  x);
            snprintf(specular, sizeof(specular), "point_lights[%d].specular", x);
            snprintf(constant, sizeof(constant), "point_lights[%d].constant", x);
            snprintf(linear,   sizeof(linear),   "point_lights[%d].linear",   x);
            snprintf(quadratic,sizeof(quadratic),"point_lights[%d].quadratic",x);

            set_uniform(shader, position,  p);
            set_uniform(shader, ambient,   light_color*0.1f);
            set_uniform(shader, diffuse,   light_color);
            set_uniform(shader, specular,  0.8f, 0.8f, 0.8f);
            set_uniform(shader, constant,  1.0f);
            set_uniform(shader, linear,    0.09f);
            set_uniform(shader, quadratic, 0.032f);
        }
    }
    set_uniform(shader, "material.shininess", (f32)32);

}

internal void
framebuffer_size_callback(i32 width, i32 height)
{
    glViewport(0, 0, width, height);
}

u32 get_vao_without_ebo(Vertex_Buffers *vabo, f32 *vertices, u32 n_verts)
{
    u32 vao_index = vabo->n_vao;
    vabo->count[vabo->n_vao] = n_verts;

    glGenVertexArrays(1, &vabo->vao[vabo->n_vao]);
    glGenBuffers(1, &vabo->vbo[vabo->n_vbo]);

    glBindBuffer(GL_ARRAY_BUFFER, vabo->vbo[vabo->n_vbo++]);
    glBufferData(GL_ARRAY_BUFFER, n_verts*sizeof(*vertices + 0), vertices, GL_STATIC_DRAW);

    glBindVertexArray(vabo->vao[vabo->n_vao++]);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    return vao_index;
}

u32 get_vao_with_ebo(Vertex_Buffers *vabo, f32 *vertices, u32 n_verts, u32 *indices, u32 n_indices)
{
    u32 vao_index = vabo->n_vao;
    vabo->count[vao_index] = n_indices;

    glGenVertexArrays(1, &vabo->vao[vabo->n_vao]);

    glBindVertexArray(vabo->vao[vabo->n_vao++]);

    glGenBuffers(1, &vabo->vbo[vabo->n_vbo]);
    glBindBuffer(GL_ARRAY_BUFFER, vabo->vbo[vabo->n_vbo++]);
    glBufferData(GL_ARRAY_BUFFER, n_verts*sizeof(*vertices + 0), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &vabo->ebo[vabo->n_ebo]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vabo->ebo[vabo->n_ebo++]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_indices*sizeof(*indices + 0), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    return vao_index;
}

internal u32
upload_to_gpu(unsigned char* pixels, int width, int height, int channels)
{
    u32 texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture_id;
}

internal u32
load_embedded_texture(cgltf_image* image)
{
    if (!image->buffer_view) return 0;

    unsigned char* data_ptr = (unsigned char*)image->buffer_view->buffer->data;
    unsigned char* offset_ptr = data_ptr + image->buffer_view->offset;
    size_t size = image->buffer_view->size;

    int width, height, channels;
    unsigned char* pixels = stbi_load_from_memory(offset_ptr, (int)size, &width, &height, &channels, 4);

    u32 texture_id = 0;
    if (pixels) {
        texture_id = upload_to_gpu(pixels, width, height, 4);
        stbi_image_free(pixels);
    } else {
        printf("Failed to load embedded texture from GLB buffer.\n");
    }

    return texture_id;
}

u32 get_texture(char *texture_path, u32 wrapping_method)
{
    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    i32 image_width, image_height, nr_channels;
    u8 *data = stbi_load(texture_path, &image_width, &image_height, &nr_channels, 0);
    stbi_set_flip_vertically_on_load(true);
    if(data)
    {
        GLenum format = 0;
        if(nr_channels == 1)
            format = GL_RED;
        else if(nr_channels == 3)
            format = GL_RGB;
        else if(nr_channels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping_method);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping_method);

        //f32 bColor[] = {1.0f, 0.0f, 0.0f, 1.0f};
        //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, bColor);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    else
    {
        log_message("[Texture] Failed to load texture %s", texture_path);
        stbi_image_free(data);
    }

    return texture;
}

u32 generate_height_map(Vertex_Buffers *vabo, Memory_Arena *arena, const char *height_map_path, Texture *texture, u32 wrapping_method)
{
    u32 vao_index = 0;

    glGenTextures(1, &texture->index);
    glBindTexture(GL_TEXTURE_2D, texture->index);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping_method);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping_method);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int nr_channels;
    u8 *data = stbi_load(height_map_path, &texture->width, &texture->height, &nr_channels, 0);
    stbi_set_flip_vertically_on_load(true);
    int image_width = texture->width;
    int image_height = texture->height;
    if(data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        //glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);

        glPatchParameteri(GL_PATCH_VERTICES, 4);

        vabo->rez = 20;
        u32 n_verts = 20*vabo->rez*vabo->rez;

        GLfloat *vertices = push_array(arena, n_verts, GLfloat);
        GLfloat *vertices_ptr = vertices;

        for(u32 i = 0; i <= vabo->rez-1; ++i)
        {
            for(u32 j = 0; j <= vabo->rez-1; ++j)
            {
                *vertices_ptr++ = -image_width/2.0f + image_width*i/(float)vabo->rez; // v.x
                *vertices_ptr++ = 0.0f; // v.y
                *vertices_ptr++ = -image_height/2.0f + image_height*j/(float)vabo->rez; // v.z
                *vertices_ptr++ = i / (float)vabo->rez; // u
                *vertices_ptr++ = j / (float)vabo->rez; // v

                *vertices_ptr++ = -image_width/2.0f + image_width*(i+1)/(float)vabo->rez; // v.x
                *vertices_ptr++ = 0.0f; // v.y
                *vertices_ptr++ = -image_height/2.0f + image_height*j/(float)vabo->rez; // v.z
                *vertices_ptr++ = (i+1) / (float)vabo->rez; // u
                *vertices_ptr++ = j / (float)vabo->rez; // v

                *vertices_ptr++ = -image_width/2.0f + image_width*i/(float)vabo->rez; // v.x
                *vertices_ptr++ = 0.0f; // v.y
                *vertices_ptr++ = -image_height/2.0f + image_height*(j+1)/(float)vabo->rez; // v.z
                *vertices_ptr++ = i / (float)vabo->rez; // u
                *vertices_ptr++ = (j+1) / (float)vabo->rez; // v

                *vertices_ptr++ = -image_width/2.0f + image_width*(i+1)/(float)vabo->rez; // v.x
                *vertices_ptr++ = 0.0f; // v.y
                *vertices_ptr++ = -image_height/2.0f + image_height*(j+1)/(float)vabo->rez; // v.z
                *vertices_ptr++ = (i+1) / (float)vabo->rez; // u
                *vertices_ptr++ = (j+1) / (float)vabo->rez; // v
            }
        }

        vao_index = vabo->n_vao;
        vabo->count[vabo->n_vao] = n_verts;

        glGenVertexArrays(1, &vabo->vao[vabo->n_vao]);
        glGenBuffers(1, &vabo->vbo[vabo->n_vbo]);

        glBindBuffer(GL_ARRAY_BUFFER, vabo->vbo[vabo->n_vbo++]);
        glBufferData(GL_ARRAY_BUFFER, n_verts*sizeof(*vertices + 0), vertices, GL_STATIC_DRAW);

        glBindVertexArray(vabo->vao[vabo->n_vao++]);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)(3*sizeof(GL_FLOAT)));
        glEnableVertexAttribArray(1);
    }
    else
    {
        log_message("[Texture] Failed to load heightMap %s", height_map_path);
        stbi_image_free(data);
    }


    return vao_index;
}

u32 generate_height_map_cpu(Vertex_Buffers *vabo, Memory_Arena *arena, char *height_map_path)
{
    u32 vao_index = 0;
    i32 image_width, image_height, nr_channels;
    stbi_us *data = stbi_load_16(height_map_path, &image_width, &image_height, &nr_channels, 0);
    if(data)
    {
        i32 n_verts = image_width * image_height * 3;
        GLfloat *vertices = push_array(arena, n_verts, GLfloat);
        GLfloat *vertices_ptr = vertices;

        f32 y_scale = 100.0f / 65535.0f;
        f32 y_shift = 0.0025f;
        for(i32 y = 0; y < image_height; ++y)
        {
            for(i32 x = 0; x < image_width; ++x)
            {
                stbi_us texel = *(data + ((x + (y * image_width)) * nr_channels));

                *vertices_ptr++ = -image_width / 2.0f + x;
                *vertices_ptr++ = texel * y_scale - y_shift;
                *vertices_ptr++ = -image_height / 2.0f + y;
            }
        }
        stbi_image_free(data);

        i32 n_indices = (image_width * 2) * (image_height-1);
        u32 *indices = push_array(arena, n_indices, u32);
        u32 *indices_ptr = indices;
        for(int y = 0; y < image_height-1; y++)
        {
            for(int x = 0; x < image_width; x++)
            {
                for(int k = 0; k < 2; k++)
                {
                    *indices_ptr++ = (x + image_width * (y + k));
                }
            }
        }

        vabo->num_strips = image_height - 1;
        vabo->num_verts_per_strip = image_width * 2;

        vao_index = vabo->n_vao;
        vabo->count[vao_index] = n_indices;

        glGenVertexArrays(1, &vabo->vao[vabo->n_vao]);
        glBindVertexArray(vabo->vao[vabo->n_vao++]);

        glGenBuffers(1, &vabo->vbo[vabo->n_vbo]);
        glBindBuffer(GL_ARRAY_BUFFER, vabo->vbo[vabo->n_vbo++]);
        glBufferData(GL_ARRAY_BUFFER, n_verts*sizeof(vertices[0]), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GL_FLOAT), (void*)0);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &vabo->ebo[vabo->n_ebo]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vabo->ebo[vabo->n_ebo++]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_indices*sizeof(indices[0]), indices, GL_STATIC_DRAW);
    }
    else
    {
        log_message("[Texture] Failed to load heightMap %s", height_map_path);
        stbi_image_free(data);
    }

    return vao_index;
}


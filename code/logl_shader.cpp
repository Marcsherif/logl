internal u32
Shader(SDL_Window *window, const char *vertexPath, const char *fragmentPath)
{
    // TODO: Load Files and error check, and do hot realoding, and do a hash
    SDL_IOStream *vertFile = SDL_IOFromFile(vertexPath, "rb");
    SDL_IOStream *fragFile = SDL_IOFromFile(fragmentPath, "rb");

    const i64 vertFileSize = SDL_GetIOSize(vertFile);
    const i64 fragFileSize = SDL_GetIOSize(fragFile);

    char *vertPtr = (char *)VirtualAlloc(0, vertFileSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    char *fragPtr = (char *)VirtualAlloc(0, fragFileSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    size_t vertFileRead = SDL_ReadIO(vertFile, vertPtr, vertFileSize);
    size_t fragFileRead = SDL_ReadIO(fragFile, fragPtr, fragFileSize);

    SDL_CloseIO(vertFile);
    SDL_CloseIO(fragFile);

    u32 vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertPtr, NULL);
    glCompileShader(vertexShader);

    i32  success;
    char infoLog[1024];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), NULL, infoLog);
        log(infoLog);
        err(infoLog);
    }
    log("[Vertex Shader] %s Initialized", vertexPath);

    u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragPtr, NULL);
    glCompileShader(fragmentShader);

    success = 0;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragmentShader, sizeof(infoLog), NULL, infoLog);
        log(infoLog);
        err(infoLog);
    }
    log("[Fragment Shader] %s Initialized", fragmentPath);

    u32 shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    success = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
        log(infoLog);
        err(infoLog);
    }
    log("[Shader] linking successful");

    VirtualFree(vertPtr, 0, MEM_RELEASE);
    VirtualFree(fragPtr, 0, MEM_RELEASE);

    return shaderProgram;
}

internal void
UseShader(u32 Id)
{
    glUseProgram(Id);
}

internal void
SetUniformb(u32 Id, const char *name, b32 value)
{
    glUniform1i(glGetUniformLocation(Id, name), (int)value);
}

internal void
SetUniform(u32 Id, const char *name, i32 value)
{
    glUniform1i(glGetUniformLocation(Id, name), value);
}

internal void
SetUniform(u32 Id, const char *name, f32 value)
{
    glUniform1f(glGetUniformLocation(Id, name), value);
}

internal void
SetUniform(u32 Id, const char *name, glm::vec3 value)
{
    glUniform3f(glGetUniformLocation(Id, name), value.x, value.y, value.z);
}

internal void
SetUniform(u32 Id, const char *name, f32 x, f32 y, f32 z)
{
    glUniform3f(glGetUniformLocation(Id, name), x, y, z);
}

internal void
SetUniform(u32 Id, const char *name, glm::mat4 value)
{
    glUniformMatrix4fv(glGetUniformLocation(Id, name), 1, GL_FALSE,
                       glm::value_ptr(value));
}

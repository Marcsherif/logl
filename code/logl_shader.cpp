internal u32
Shader(SDL_Window *window, const char *vertexPath, const char *fragmentPath,
       const char *tcsPath = 0, const char *tesPath = 0)
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

    GLuint tcsShader = 0;
    GLuint tesShader = 0;
    char *tcsPtr = 0;
    char *tesPtr = 0;
    if((tcsPath && tesPath) != 0 )
    {
        SDL_IOStream *tcsFile = SDL_IOFromFile(tcsPath, "rb");
        SDL_IOStream *tesFile = SDL_IOFromFile(tesPath, "rb");

        const i64 tcsFileSize = SDL_GetIOSize(tcsFile);
        const i64 tesFileSize = SDL_GetIOSize(tesFile);

        tcsPtr = (char *)VirtualAlloc(0, tcsFileSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        tesPtr = (char *)VirtualAlloc(0, tesFileSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

        size_t tcsFileRead = SDL_ReadIO(tcsFile, tcsPtr, tcsFileSize);
        size_t tesFileRead = SDL_ReadIO(tesFile, tesPtr, tesFileSize);

        SDL_CloseIO(tcsFile);
        SDL_CloseIO(tesFile);

        tcsShader = glCreateShader(GL_TESS_CONTROL_SHADER);
        glShaderSource(tcsShader, 1, &tcsPtr, NULL);
        glCompileShader(tcsShader);

        tesShader = glCreateShader(GL_TESS_EVALUATION_SHADER);
        glShaderSource(tesShader, 1, &tesPtr, NULL);
        glCompileShader(tesShader);
    }

    SDL_CloseIO(vertFile);
    SDL_CloseIO(fragFile);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertPtr, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragPtr, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    if((tcsPath && tesPath) != 0 )
    {
        glAttachShader(shaderProgram, tcsShader);
        glAttachShader(shaderProgram, tesShader);
    }
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint linked = false;
    GLint compiled = false;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiled);
    if((tcsPath && tesPath) != 0 )
    {
        glGetShaderiv(tcsShader, GL_COMPILE_STATUS, &compiled);
        glGetShaderiv(tesShader, GL_COMPILE_STATUS, &compiled);
    }
    if(!(linked && compiled))
    {
        char vertexErrors[4096];
        char fragmentErrors[4096];
        char programErrors[4096];
        glGetShaderInfoLog(vertexShader, sizeof(vertexErrors), NULL, vertexErrors);
        glGetShaderInfoLog(fragmentShader, sizeof(fragmentErrors), NULL, fragmentErrors);
        glGetProgramInfoLog(shaderProgram, sizeof(programErrors), NULL, programErrors);

        Log(vertexErrors);
        Log(fragmentErrors);
        Log(programErrors);
        if((tcsPath && tesPath) != 0 )
        {
            char tcsErrors[4096];
            char tesErrors[4096];
            glGetShaderInfoLog(tcsShader, sizeof(tcsErrors), NULL, tcsErrors);
            glGetShaderInfoLog(tesShader, sizeof(tesErrors), NULL, tesErrors);
            Log(tcsErrors);
            Log(tesErrors);
        }
        err("shader error");
    }
    Log("[Vertex Shader] %s Initialized", vertexPath);
    Log("[Fragment Shader] %s Initialized", fragmentPath);
    if((tcsPath && tesPath) != 0 )
    {
        Log("[TCS Shader] %s Initialized", tcsPath);
        Log("[TES Shader] %s Initialized", tesPath);
        VirtualFree(tcsPtr, 0, MEM_RELEASE);
        VirtualFree(tesPtr, 0, MEM_RELEASE);
    }
    Log("[Shader] linking successful");

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
SetUniform(u32 Id, const char *name, f32 x, f32 y)
{
    glUniform2f(glGetUniformLocation(Id, name), x, y);
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

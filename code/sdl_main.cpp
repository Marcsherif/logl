#ifndef LOGL_ROOT
#pragma once
#include "root.unity.h"
#endif

#include <SDL3/SDL.h>
#include <glad/glad.h>

#include "glad.c"
#include "logl.h"

#define err(msg) Quit(window, context); ThrowError(msg);

struct myWindow
{
    b32 updateCaption = false;
    i32 Width;
    i32 Height;
    b32 FullScreen = false;
    b32 Minimized = false;
    b32 MouseFocus;
    b32 KeyboardFocus;
};

struct VABO
{
    u32 *VAO;
    // NOTE(marc): If using an EBO this is index count else its vertex count
    u32 *count;
    u32 nVAO;

    u32 *VBO;
    u32 nVBO;

    u32 *EBO;
    u32 nEBO;
};

void PaintMyWindow()
{
}

void FramebufferSizeCallback(i32 width, i32 height)
{
    glViewport(0, 0, width, height);
}

void Quit(SDL_Window *window, SDL_GLContext context)
{
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

b32 ProcessInput(SDL_Window* window, SDL_GLContext context, SDL_Event e, myWindow *myWindow)
{
    b32 quit = false;
    if(e.type == SDL_EVENT_KEY_DOWN)
    {
        switch(e.key.key)
        {
            case SDLK_RETURN:
            {
                if(myWindow->FullScreen)
                {
                    SDL_SetWindowFullscreen(window, 0);
                    myWindow->FullScreen = false;
                }
                else
                {
                    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
                    myWindow->FullScreen = true;
                    myWindow->Minimized = false;
                }
            } break;

            case SDLK_ESCAPE:
            {
                quit = true;
                Quit(window, context);
            } break;
        }
    }

    return quit;
}

void HandleEvent(SDL_Window* window, SDL_Event e, myWindow *myWindow)
{
    switch(e.type)
    {
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            myWindow->Width = e.window.data1;
            myWindow->Height = e.window.data2;
            FramebufferSizeCallback(myWindow->Width, myWindow->Height);
            PaintMyWindow();
            break;

        case SDL_EVENT_WINDOW_EXPOSED:
            PaintMyWindow();
            break;

        case SDL_EVENT_WINDOW_MOUSE_ENTER:
            myWindow->MouseFocus = true;
            myWindow->updateCaption = true;
            break;

        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
            myWindow->MouseFocus = false;
            myWindow->updateCaption = true;
            break;

        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            myWindow->KeyboardFocus = true;
            myWindow->updateCaption = true;
            break;

        case SDL_EVENT_WINDOW_FOCUS_LOST:
            myWindow->KeyboardFocus = false;
            myWindow->updateCaption = true;
            break;

        case SDL_EVENT_WINDOW_MINIMIZED:
            myWindow->Minimized = true;
            break;

        case SDL_EVENT_WINDOW_MAXIMIZED:
            myWindow->Minimized = false;
            break;

        case SDL_EVENT_WINDOW_RESTORED:
            myWindow->Minimized = false;
            break;
    }
    if(myWindow->updateCaption)
    {
        char caption[120];
        if(myWindow->MouseFocus) strcat_s(caption, "SDL Tutorial - MouseFocus: On"); else strcat_s(caption, "SDL Tutorial - MouseFocus: Off");
        if(myWindow->KeyboardFocus) strcat_s(caption,"  KeyboardFocus: On"); else strcat_s(caption,"  KeyboardFocus: Off");
        SDL_SetWindowTitle(window, caption);
    }
}

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragmentShaderSourceOrange = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\0";

const char *fragmentShaderSourceRed = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(0.8f, 0.0f, 0.0f, 1.0f);\n"
    "}\0";

u32 GetShader(SDL_Window *window, SDL_GLContext context, const char *fragmentShaderSource)
{
    u32 vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    i32  success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), NULL, infoLog);
        err("ERROR::SHADER::VERTEX::COMPILATION_FAILED");
    }

    u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    success = 0;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragmentShader, sizeof(infoLog), NULL, infoLog);
        err("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED");
    }

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
        err("ERROR::LINKING::SHADERS::FAILED");
    }

    return shaderProgram;
}

f32 triangleVertices[] = {
    -0.8f, -0.8f, 0.0f,
    0.0f, 0.0f, 0.0f,
    -0.4f,  0.2f, 0.0f,

    0.8f, 0.8f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.4f,  -0.2f, 0.0f
};

f32 triangleVertices1[] = {
    -0.8f, -0.8f, 0.0f,
    0.0f, 0.0f, 0.0f,
    -0.4f,  0.2f, 0.0f
};

f32 triangleVertices2[] = {
    0.8f, 0.8f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.4f,  -0.2f, 0.0f
};

u32 GetVAOwithoutEBO(VABO *vabo, f32 *vertices, u32 nVerts)
{
    u32 vaoIndex = vabo->nVAO;
    vabo->count[vabo->nVAO] = nVerts;

    glGenVertexArrays(1, &vabo->VAO[vabo->nVAO]);

    glBindVertexArray(vabo->VAO[vabo->nVAO++]);

    glGenBuffers(1, &vabo->VBO[vabo->nVBO]);
    glBindBuffer(GL_ARRAY_BUFFER, vabo->VBO[vabo->nVBO++]);
    glBufferData(GL_ARRAY_BUFFER, nVerts*sizeof(*vertices + 0), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    return vaoIndex;
}

f32 squareVertices[] = {
    0.5f,  0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f
};
u32 squareIndices[] = {
    0, 1, 3,
    1, 2, 3
};

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    return vaoIndex;
}

int main()
{
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        ThrowError("Failed to initialized SDL3!");
    }

    i32 width = 1920;
    i32 height = 1080;
    SDL_Window *window = SDL_CreateWindow("Hello, world!", width, height,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext context = 0;

    u32 shapeCount = 4;
    u64 memorySize = sizeof(VABO)*shapeCount;
    void *memory = (void *)VirtualAlloc(0, memorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    memory_arena arena = {};
    InitializeArena(&arena, memorySize, (u8 *)memory);

    VABO vabo = {};
    vabo.VAO = PushArray(&arena, 4, u32);
    vabo.count = PushArray(&arena, 4, u32);
    vabo.VBO = PushArray(&arena, 4, u32);
    vabo.EBO = PushArray(&arena, 4, u32);

    u32 orangeShader = 0;
    u32 yellowShader = 0;
    if(!window)
    {
        ThrowError("Failed to initialized Window!");
    }
    else
    {
        context = SDL_GL_CreateContext(window);
        if(!context)
        {
            err("Failed to initialize Context");
        }
        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        {
            err("Failed to initialize GLAD");
        }

        glViewport(0, 0, width, height);

        orangeShader = GetShader(window, context, fragmentShaderSourceOrange);
        yellowShader = GetShader(window, context, fragmentShaderSourceRed);

        u32 nVerts1 = ArrayCount(triangleVertices1);
        u32 triangle1 = GetVAOwithoutEBO(&vabo, triangleVertices1, nVerts1);

        u32 nVerts2 = ArrayCount(triangleVertices2);
        u32 triangle2 = GetVAOwithoutEBO(&vabo, triangleVertices2, nVerts2);

        u32 nSquareVerts = ArrayCount(squareVertices);
        u32 nIndices = ArrayCount(squareIndices);
        u32 rectangle = GetVAOwithEBO(&vabo, squareVertices, nSquareVerts,
                                      squareIndices, nIndices);

        myWindow myWindow;
        SDL_Event e;
        b32 quit = false;
        while(!quit)
        {
            while(SDL_PollEvent(&e))
            {
                if(e.type == SDL_EVENT_QUIT) quit = true;

                HandleEvent(window, e, &myWindow);
                quit = ProcessInput(window, context, e, &myWindow);
            }

            if(!myWindow.Minimized)
            {
                glClearColor(0x18 / 255.0f, 0x18 / 255.0f, 0x18 / 255.0f, 0xFF);
                glClear(GL_COLOR_BUFFER_BIT);

#if 1
                glUseProgram(orangeShader);
                glBindVertexArray(vabo.VAO[rectangle]);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glDrawElements(GL_TRIANGLES, vabo.count[rectangle], GL_UNSIGNED_INT, 0);

                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glUseProgram(orangeShader);
                glBindVertexArray(vabo.VAO[triangle1]);
                glDrawArrays(GL_TRIANGLES, 0, vabo.count[triangle1]);

                glUseProgram(yellowShader);
                glBindVertexArray(vabo.VAO[triangle2]);
                glDrawArrays(GL_TRIANGLES, 0, vabo.count[triangle2]);
#endif

                SDL_GL_SwapWindow(window);
            }
        }

    }

    // TODO: de-allocate resources
    glDeleteVertexArrays(vabo.nVAO, vabo.VAO);
    glDeleteBuffers(vabo.nVBO, vabo.VBO);
    glDeleteProgram(orangeShader);
    glDeleteProgram(yellowShader);

    Quit(window, context);
    return 0;
}

#ifndef LOGL_ROOT
#pragma once
#include "root.unity.h"
#endif

#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <glad/glad.c>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include "logl.h"

void Quit(SDL_Window *window, SDL_GLContext context);
#define err(msg) Quit(window, context); ThrowError(msg);

global_variable b32 globalQuit = false;
global_variable b32 firstMouse = true;
global_variable f32 alpha = 0;
global_variable f64 FPS = 0;

#include "logl_shader.cpp"
#include "logl_camera.cpp"

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

internal void
FramebufferSizeCallback(i32 width, i32 height)
{
    glViewport(0, 0, width, height);
}

void
Quit(SDL_Window *window, SDL_GLContext context)
{
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

internal void
ProcessKeyboardMessage(game_button_state *newState, b32 isDown)
{
    if(newState->endedDown != isDown)
    {
        newState->endedDown = isDown;
        ++newState->halfTransitionCount;
    }
}

internal void
ProcessInput(my_window *myWindow, game_controller_input *keyboard, SDL_GLContext context, SDL_Event e)
{
    if(e.type == SDL_EVENT_QUIT) globalQuit = true;
    if(e.type == SDL_EVENT_KEY_UP || e.type == SDL_EVENT_KEY_DOWN)
    {
        //SDL_Keycode key = e.key.key;
        const bool *key = SDL_GetKeyboardState(NULL);
        b32 isDown = 0;
        switch(e.key.type)
        {
            case SDL_EVENT_KEY_DOWN:
            {
                isDown = 1;
            } break;
            case SDL_EVENT_KEY_UP:
            {
                isDown = 0;
            } break;
        }

        if(key[SDL_SCANCODE_RETURN])
        {
            if(myWindow->fullScreen)
            {
                SDL_SetWindowFullscreen(myWindow->window, 0);
                myWindow->fullScreen = false;
            }
            else
            {
                SDL_SetWindowFullscreen(myWindow->window, SDL_WINDOW_FULLSCREEN);
                myWindow->fullScreen = true;
                myWindow->minimized = false;
            }
        }

        if(key[SDL_SCANCODE_ESCAPE])
        {
            log("ESCAPE PRESSED");
            globalQuit = true;
            Quit(myWindow->window, context);
        }

        if(key[SDL_SCANCODE_W])
        {
            ProcessKeyboardMessage(&keyboard->moveForward, isDown);
        }

        if(key[SDL_SCANCODE_S])
        {
            ProcessKeyboardMessage(&keyboard->moveBackward, isDown);
        }

        if(key[SDL_SCANCODE_A])
        {
            ProcessKeyboardMessage(&keyboard->moveLeft, isDown);
        }

        if(key[SDL_SCANCODE_D])
        {
            ProcessKeyboardMessage(&keyboard->moveRight, isDown);
        }

        if(key[SDL_SCANCODE_DOWN])
        {
            alpha -= 0.1;
        }

        if(key[SDL_SCANCODE_UP])
        {
            alpha += 0.1;
        }

        if(key[SDL_SCANCODE_Z])
        {
            ProcessKeyboardMessage(&keyboard->zoomin, isDown);
        }

        if(key[SDL_SCANCODE_X])
        {
            ProcessKeyboardMessage(&keyboard->zoomout, isDown);
        }
    }
}

void HandleEvent(my_window *myWindow, SDL_Event e, my_camera camera)
{
    switch(e.type)
    {
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            myWindow->width = e.window.data1;
            myWindow->height = e.window.data2;
            FramebufferSizeCallback(myWindow->width, myWindow->height);
            PaintMyWindow();
            break;

        case SDL_EVENT_WINDOW_EXPOSED:
            PaintMyWindow();
            break;

        case SDL_EVENT_WINDOW_MOUSE_ENTER:
            myWindow->mouseFocus = true;
            myWindow->updateCaption = true;
            log("mouse focused!");
            break;

        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
            myWindow->mouseFocus = false;
            myWindow->updateCaption = true;
            break;

        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            myWindow->keyboardFocus = true;
            myWindow->updateCaption = true;
            break;

        case SDL_EVENT_WINDOW_FOCUS_LOST:
            myWindow->keyboardFocus = false;
            myWindow->updateCaption = true;
            break;

        case SDL_EVENT_WINDOW_MINIMIZED:
            myWindow->minimized = true;
            break;

        case SDL_EVENT_WINDOW_MAXIMIZED:
            myWindow->minimized = false;
            break;

        case SDL_EVENT_WINDOW_RESTORED:
            myWindow->minimized = false;
            break;
    }
    if(myWindow->updateCaption)
    {
        const u32 bufSize = 320;
        char caption[bufSize] = {};
        if(myWindow->mouseFocus)
            SDL_strlcat(caption, "SDL Tutorial - MouseFocus: On", bufSize);
        else
            SDL_strlcat(caption, "SDL Tutorial - MouseFocus: Off", bufSize);
        if(myWindow->keyboardFocus)
            SDL_strlcat(caption,"  KeyboardFocus: On", bufSize);
        else
            SDL_strlcat(caption,"  KeyboardFocus: Off", bufSize);
        char fovText[30] = {};
        SDL_snprintf(fovText, 30, "  Fov: %.02f  FPS: %.02ff/s", camera.fov, FPS);
        SDL_strlcat(caption, fovText, bufSize);
        SDL_SetWindowTitle(myWindow->window, caption);
    }
}

f32 triangleVertices[] = {
    -0.8f, -0.8f, 0.0f,  8.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,    0.0f, 2.0f, 0.0f,
    -0.4f,  0.2f, 0.0f,  0.0f, 0.0f, 0.0f,

    0.8f, 0.8f, 0.0f,  8.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,  0.0f, 2.0f, 0.0f,
    0.4f,  -0.2f, 0.0,  0.0f, 0.0f, 0.0f
};

f32 triangleVertices1[] = {
    -0.8f, -0.8f, 0.0f,    1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 0.0f,
    -0.4f,  0.2f, 0.0f,    0.0f, 0.0f, 0.0f
};


f32 middleTriangleVertices[] = {
    // positions         // colors
     0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
     0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top
};

f32 texCoords[] = {
    0.0f, 0.0f,  // lower-left corner
    1.0f, 0.0f,  // lower-right corner
    0.5f, 1.0f   // top-center corner
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    return vaoIndex;
}

f32 verticesWNormals[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

f32 textureVertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};
glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    return vaoIndex;
}

u32 GetTexture(char *texturePath, u32 format, u32 wrappingMethod)
{
    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrappingMethod);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrappingMethod);

    //f32 bColor[] = {1.0f, 0.0f, 0.0f, 1.0f};
    //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, bColor);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    i32 imageWidth, imageHeight, nrChannels;
    u8 *data = stbi_load(texturePath, &imageWidth, &imageHeight, &nrChannels, 0);
    stbi_set_flip_vertically_on_load(true);
    if(data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else
    {
        log("Failed to load texture");
    }

    return texture;
}

f32 GetAlphaBlend(f32 frequency)
{
    f32 time = (f32)SDL_GetPerformanceCounter() / (f32)SDL_GetPerformanceFrequency();
    f32 alphaBlend = ((f32)sin(time * frequency) + 1.0f) / 2.0f;
    return alphaBlend;
}

f32 GetSecondsElapsed(u64 lastCycleCount, u64 frequency)
{
    u64 currTime = SDL_GetPerformanceCounter();
    f32 timeValue = (f32)((currTime - lastCycleCount) / (f32)frequency);
    return timeValue;
}

void PythagoreanTree(SDL_Window *window, glm::mat4 transform, VABO vabo, u32 object, int depth, u32 shader)
{
    if (depth == 0) return;

    float scaling = 0.70710678f;

    SetUniform(shader, "transform", transform);
    glDrawElements(GL_TRIANGLES, vabo.count[object], GL_UNSIGNED_INT, 0);

    glm::mat4 leftTransform = transform;
    leftTransform = glm::translate(leftTransform, glm::vec3(-0.5f, 1.0f, 0.0f));
    leftTransform = glm::rotate(leftTransform, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    leftTransform = glm::scale(leftTransform, glm::vec3(scaling, scaling, 1.0f));

    SetUniform(shader, "transform", leftTransform);
    glDrawElements(GL_TRIANGLES, vabo.count[object], GL_UNSIGNED_INT, 0);

    glm::mat4 rightTransform = transform;
    rightTransform = glm::translate(rightTransform, glm::vec3(0.5f, 1.0f, 0.0f));
    rightTransform = glm::rotate(rightTransform, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    rightTransform = glm::scale(rightTransform, glm::vec3(scaling, scaling, 1.0f));

    SetUniform(shader, "transform", rightTransform);
    glDrawElements(GL_TRIANGLES, vabo.count[object], GL_UNSIGNED_INT, 0);

    PythagoreanTree(window, leftTransform, vabo, object, depth - 1, shader);
    PythagoreanTree(window, rightTransform, vabo, object, depth - 1, shader);
}

void DrawPythagoreanTree(SDL_Window *window, glm::mat4 initialTransform, VABO vabo, u32 object, int maxDepth, u32 shader)
{
    for (int depth = 0; depth < maxDepth; ++depth) {
        glm::mat4 transform = initialTransform;
        SetUniform(shader, "transform", transform);

        // NOTE(marc): bad implementation, calculates the entire thing at every level
        PythagoreanTree(window, transform, vabo, object, depth, shader);

        SDL_Delay(10);

        SDL_GL_SwapWindow(window);
    }
}

int main()
{
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        ThrowError("Failed to initialized SDL3!");
    }

#define width 1920
#define height 1080
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

    u32 lightShader = 0;
    u32 sourceLightShader = 0;
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
        SDL_SetWindowRelativeMouseMode(window, 1);

        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        {
            err("Failed to initialize GLAD");
        }

        game_input input[2] {};
        game_input *newInput = &input[0];
        game_input *oldInput = &input[1];

        glViewport(0, 0, width, height);

        char *vertShaderPath = "../shaders/light.vs";
        char *fragShaderPath = "../shaders/light.fs";
        lightShader = Shader(window, context, vertShaderPath, fragShaderPath);

        fragShaderPath = "../shaders/lightSource.fs";
        sourceLightShader = Shader(window, context, vertShaderPath, fragShaderPath);
        glm::vec3 lightPos(1.2f, 0.0f, 2.0f);

        u32 nVerts1 = ArrayCount(middleTriangleVertices);
        u32 triangle1Verts = GetVAOwithoutEBO(&vabo, middleTriangleVertices, nVerts1);

        u32 nVerts2 = ArrayCount(triangleVertices1);
        u32 triangle2 = GetVAOwithoutEBO(&vabo, triangleVertices1, nVerts2);

        u32 nSquareVerts = ArrayCount(verticesWNormals);
        u32 rectangle = GetVAOwithoutEBO(&vabo, verticesWNormals, nSquareVerts);
        u32 lightSourceCube = GetVAOwithoutEBO(&vabo, verticesWNormals, nSquareVerts);
        glEnable(GL_DEPTH_TEST);

        u32 container = GetTexture("../data/container.jpg", GL_RGB, GL_MIRRORED_REPEAT);
        u32 awesomeFace = GetTexture("../data/awesomeface.png", GL_RGBA, GL_MIRRORED_REPEAT);
        UseShader(lightShader);

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), f32(width / height), 0.1f, 100.0f);

        glm::vec3 camPos   = glm::vec3(0.0f, 0.0f,  3.0f);
        glm::vec3 camUp    = glm::vec3(0.0f, 1.0f,  0.0f);
        my_camera debugCamera = InitCamera(camPos, camUp);

        i32 nrAttributes;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
        log("Maximum nr of vertex attributes supported: %d\n", nrAttributes);

        my_window myWindow;
        myWindow.window = window;
        SDL_Event e;
        u64 upTime = SDL_GetPerformanceCounter();
        u64 frequency = SDL_GetPerformanceFrequency();
        u64 lastCycleCount = upTime;
        u64 cyclesElapsed = lastCycleCount;
        f64 MCPF = 0;
        f32 lastMouseX = width/2;
        f32 lastMouseY = height/2;
        i32 movement = 0;
        while(!globalQuit)
        {
            newInput->dtForFrame = (f32)MCPF;

            game_controller_input *oldKeyboard = GetController(oldInput, 0);
            game_controller_input *newKeyboard = GetController(newInput, 0);
            *newKeyboard = {};
            newKeyboard->isConnected = true;
            for(i32 buttonIndex = 0;
                buttonIndex < ArrayCount(newKeyboard->buttons);
                ++buttonIndex)
            {
                newKeyboard->buttons[buttonIndex].endedDown =
                    oldKeyboard->buttons[buttonIndex].endedDown;
            }

            while(SDL_PollEvent(&e))
            {
                ProcessInput(&myWindow, newKeyboard, context, e);
                HandleEvent(&myWindow, e, debugCamera);
            }

            //log("A BUTTON : %d", newKeyboard->moveLeft.endedDown);
            ProcessCameraInputs(&debugCamera, newKeyboard, newInput->dtForFrame);

            f32 mousePosX, mousePosY;
            SDL_GetMouseState(&mousePosX, &mousePosY);
            newInput->mouseX = mousePosX;
            newInput->mouseY = mousePosY;
            newInput->mouseZ = 0; // TODO(marc): Support mousewheel?
#if 0
            Win32ProcessKeyboardMessage(&newInput->mouseButtons[0],
                                        GetKeyState(VK_LBUTTON) & (1 << 15));
            Win32ProcessKeyboardMessage(&newInput->mouseButtons[1],
                                        GetKeyState(VK_MBUTTON) & (1 << 15));
            Win32ProcessKeyboardMessage(&newInput->mouseButtons[2],
                                        GetKeyState(VK_RBUTTON) & (1 << 15));
            Win32ProcessKeyboardMessage(&newInput->mouseButtons[3],
                                        GetKeyState(VK_XBUTTON1) & (1 << 15));
            Win32ProcessKeyboardMessage(&newInput->mouseButtons[4],
                                        GetKeyState(VK_XBUTTON2) & (1 << 15));
#endif

            if(!myWindow.minimized)
            {

                glClearColor(0x18 / 255.0f, 0x18 / 255.0f, 0x18 / 255.0f, 0xFF);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                f32 timeValue = GetSecondsElapsed(upTime, frequency);
                f32 ab = GetAlphaBlend(1.0f);

                UseShader(lightShader);
                SetUniform(lightShader, "objectColor", 1.0f, 0.5f, 0.31f);
                SetUniform(lightShader, "lightColor", 1.0f, 1.0f, 1.0f);
                SetUniform(lightShader, "lightPos", lightPos);
                SetUniform(lightShader, "lookDir", debugCamera.position);

                projection = glm::perspective(glm::radians(debugCamera.fov), f32(width / height), 0.1f, 100.0f);
                GetCameraDirection(&myWindow, newInput, &debugCamera, &lastMouseX, &lastMouseY);
                glm::mat4 view = GetViewMatrix(&debugCamera);

                SetUniform(lightShader, "view", view);
                SetUniform(lightShader, "projection", projection);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, container);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, awesomeFace);

                glBindVertexArray(vabo.VAO[rectangle]);
                for(u32 i = 0; i < 1; ++i)
                {
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, cubePositions[i]);
                    //f32 angle = 20.0f * i;
                    //model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
                    //if(i % 3 == 0 || i == 0)
                    //{
                    //    model = glm::rotate(model, sin(timeValue), glm::vec3(1.0f, 1.0f, 0.0f));
                    //}
                    SetUniform(lightShader, "model", model);

                    glDrawArrays(GL_TRIANGLES, 0, vabo.count[rectangle]);
                }

                UseShader(sourceLightShader);
                SetUniform(sourceLightShader, "view", view);
                SetUniform(sourceLightShader, "projection", projection);

                glm::mat4 model = glm::mat4(1.0f);
                lightPos.x = sin(timeValue)*2;
                lightPos.y = 0;//sin(timeValue)*2;
                lightPos.z = cos(timeValue)*2;
                model = glm::translate(model, lightPos);
                model = glm::scale(model, glm::vec3(0.2f));
                SetUniform(sourceLightShader, "model", model);

                glBindVertexArray(vabo.VAO[lightSourceCube]);
                glDrawArrays(GL_TRIANGLES, 0, vabo.count[lightSourceCube]);

                SDL_GL_SwapWindow(window);

                // TODO: track fps
                u64 endCycleCount = SDL_GetPerformanceCounter();
                cyclesElapsed = endCycleCount - lastCycleCount;
                lastCycleCount = endCycleCount;

                MCPF = (f64)cyclesElapsed / (f64)frequency;
#if 1
                FPS = 1.0f/MCPF;

                //log("%.02ff/s,  %.02fmc/f\n", FPS, MCPF);
#endif
            }
        }

    }

    // TODO: de-allocate resources
    glDeleteVertexArrays(vabo.nVAO, vabo.VAO);
    glDeleteBuffers(vabo.nVBO, vabo.VBO);
    glDeleteProgram(lightShader);
    glDeleteProgram(sourceLightShader);

    Quit(window, context);
    return 0;
}

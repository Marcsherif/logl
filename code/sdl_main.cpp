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

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include "logl.h"

void Quit(SDL_Window *window, SDL_GLContext context);
#define err(msg) Quit(window, context); ThrowError(msg);

#include "logl_shader.cpp"

global_variable b32 firstMouse = true;
global_variable f32 alpha = 0;
global_variable f32 fov = 90;
global_variable f32 FOV_MAX = 180;
global_variable f64 FPS = 0;

struct myWindow
{
    SDL_Window *window;
    b32 updateCaption = false;
    i32 width;
    i32 height;
    b32 fullScreen = false;
    b32 minimized = false;
    b32 mouseFocus;
    b32 keyboardFocus;
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

b32 ProcessInput(myWindow *myWindow, SDL_GLContext context, SDL_Event e,
                 glm::vec3 *camPos, f32 *camSpeed, glm::vec3 *camFront, glm::vec3 *camUp)
{
    b32 quit = false;
    if(e.type == SDL_EVENT_KEY_DOWN)
    {
        switch(e.key.key)
        {
            case SDLK_RETURN:
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
            } break;

            case SDLK_ESCAPE:
            {
                log("ESCAPE PRESSED");
                quit = true;
                Quit(myWindow->window, context);
            } break;

            case SDLK_W:
            {
                *camPos += *camSpeed * (*camFront);
            } break;

            case SDLK_S:
            {
                *camPos -= *camSpeed * *camFront;
            } break;

            case SDLK_A:
            {
                *camPos -= glm::normalize(glm::cross(*camFront, *camUp)) * *camSpeed;
            } break;

            case SDLK_D:
            {
                *camPos += glm::normalize(glm::cross(*camFront, *camUp)) * *camSpeed;
            } break;

            case SDLK_DOWN:
            {
                alpha -= 0.1;
            } break;

            case SDLK_UP:
            {
                alpha += 0.1;
            } break;

            case SDLK_Z:
            {
                fov -= 30;
                if(fov < 1.0f)
                    fov = 1.0f;
            } break;

            case SDLK_X:
            {
                fov += 30;
                if(fov > FOV_MAX)
                    fov = FOV_MAX;
            } break;
        }
    }

    return quit;
}

void HandleEvent(myWindow *myWindow, SDL_Event e)
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
        SDL_snprintf(fovText, 30, "  Fov: %.02f  FPS: %.02ff/s", fov, FPS);
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    return vaoIndex;
}

f32 squareVertices[] = {
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

glm::vec3
GetCameraDirection(myWindow *myWindow, f32 *lastMouseX, f32 *lastMouseY, f32 *yaw, f32 *pitch)
{
    f32 mousePosX = 0;
    f32 mousePosY = 0;
    SDL_GetMouseState(&mousePosX, &mousePosY);

    f32 width = (f32)myWindow->width;
    f32 height = (f32)myWindow->height;
    const f32 edgeThreshold = 10.0f;

    if(mousePosX <= edgeThreshold)
    {
        firstMouse = true;
        mousePosX = width-edgeThreshold-1;
        SDL_WarpMouseInWindow(myWindow->window, mousePosX, mousePosY);
    }
    if(mousePosX >= width - edgeThreshold)
    {
        firstMouse = true;
        mousePosX = edgeThreshold;
        SDL_WarpMouseInWindow(myWindow->window, mousePosX, mousePosY);
    }

    if(mousePosY <= edgeThreshold)
    {
        firstMouse = true;
        mousePosY = height-edgeThreshold-1;
        SDL_WarpMouseInWindow(myWindow->window, mousePosX, mousePosY);
    }
    if(mousePosY >= height-edgeThreshold)
    {
        firstMouse = true;
        mousePosY = edgeThreshold;
        SDL_WarpMouseInWindow(myWindow->window, mousePosX, mousePosY);
    }

    if (firstMouse)
    {
        *lastMouseX = mousePosX;
        *lastMouseY = mousePosY;
        firstMouse = false;
    }

    f32 mouseOffsetX = mousePosX - *lastMouseX;
    f32 mouseOffsetY = *lastMouseY - mousePosY;
    *lastMouseX = mousePosX;
    *lastMouseY = mousePosY;
    const f32 sensitivity = 0.1f;
    mouseOffsetX *= sensitivity;
    mouseOffsetY *= sensitivity;

    *yaw   += mouseOffsetX;
    *pitch += mouseOffsetY;

    if(*pitch > 89.0f)
        *pitch =  89.0f;
    if(*pitch < -89.0f)
        *pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(*yaw)) * cos(glm::radians(*pitch));
    direction.y = sin(glm::radians(*pitch));
    direction.z = sin(glm::radians(*yaw)) * cos(glm::radians(*pitch));
    glm::vec3 camFront = glm::normalize(direction);

    return camFront;
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

    u32 shader = 0;
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

        glViewport(0, 0, width, height);

        char *vertShaderPath = "../shaders/vert.glsl";
        char *fragShaderPath = "../shaders/frag.glsl";
        shader = Shader(window, context, vertShaderPath, fragShaderPath);

        u32 nVerts1 = ArrayCount(middleTriangleVertices);
        u32 triangle1Verts = GetVAOwithoutEBO(&vabo, middleTriangleVertices, nVerts1);

        u32 nVerts2 = ArrayCount(triangleVertices1);
        u32 triangle2 = GetVAOwithoutEBO(&vabo, triangleVertices1, nVerts2);

        u32 nSquareVerts = ArrayCount(squareVertices);
        u32 rectangle = GetVAOwithoutEBO(&vabo, squareVertices, nSquareVerts);
        glEnable(GL_DEPTH_TEST);

        u32 container = GetTexture("../data/container.jpg", GL_RGB, GL_MIRRORED_REPEAT);
        u32 awesomeFace = GetTexture("../data/awesomeface.png", GL_RGBA, GL_MIRRORED_REPEAT);
        UseShader(shader);
        SetUniform(shader, "texture1", 0);
        SetUniform(shader, "texture2", 1);

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), f32(width / height), 0.1f, 100.0f);

        glm::vec3 camPos   = glm::vec3(0.0f, 0.0f,  3.0f);
        glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 camUp    = glm::vec3(0.0f, 1.0f,  0.0f);

        f32 yaw = 0.0f;
        f32 pitch = 0.0f;

        i32 nrAttributes;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
        log("Maximum nr of vertex attributes supported: %d\n", nrAttributes);

        myWindow myWindow;
        myWindow.window = window;
        SDL_Event e;
        b32 quit = false;
        u64 upTime = SDL_GetPerformanceCounter();
        u64 lastCycleCount = upTime;
        u64 cyclesElapsed = lastCycleCount;
        f64 MCPF = 0;
        u64 frequency = SDL_GetPerformanceFrequency();
        f32 camSpeed = 0.05f;
        f32 lastMouseX = width/2;
        f32 lastMouseY = height/2;
        while(!quit)
        {
            while(SDL_PollEvent(&e))
            {
                if(e.type == SDL_EVENT_QUIT) quit = true;

                HandleEvent(&myWindow, e);
                quit = ProcessInput(&myWindow, context, e,
                                    &camPos, &camSpeed, &camFront, &camUp);
            }

            if(!myWindow.minimized)
            {
                glClearColor(0x18 / 255.0f, 0x18 / 255.0f, 0x18 / 255.0f, 0xFF);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                f32 timeValue = GetSecondsElapsed(upTime, frequency);
                camSpeed = 5.5f * (f32)MCPF;
                f32 ab = GetAlphaBlend(1.0f);

                projection = glm::perspective(glm::radians(fov), f32(width / height), 0.1f, 100.0f);
                camFront = GetCameraDirection(&myWindow, &lastMouseX, &lastMouseY, &yaw, &pitch);

                glm::mat4 view;
                camPos.y = 0;
                //view = glm::lookAt(camPos, camPos + camFront, camUp);
                // POS  TARGET  UP
                glm::vec3 camRight = glm::cross(camFront, camUp);
                view[0] = glm::vec4(camRight, 0.0f);
                view[1] = glm::vec4(camUp, 0.0f);
                view[2] = glm::vec4(camFront+camPos, 0.0f);

                glm::mat4 pos;
                pos[0][3] = -camPos.x;
                pos[1][3] = -camPos.y;
                pos[1][3] = -camPos.z;

                view = view * pos;

                UseShader(shader);
                SetUniform(shader, "alpha", 0.2f);
                SetUniform(shader, "view", view);
                SetUniform(shader, "projection", projection);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, container);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, awesomeFace);

                glBindVertexArray(vabo.VAO[rectangle]);
                for(u32 i = 0; i < 10; ++i)
                {
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, cubePositions[i]);
                    f32 angle = 20.0f * i;
                    model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
                    if(i % 3 == 0 || i == 0)
                    {
                        model = glm::rotate(model, tan(timeValue), glm::vec3(1.0f, 1.0f, 0.0f));
                    }
                    SetUniform(shader, "model", model);

                    glDrawArrays(GL_TRIANGLES, 0, vabo.count[rectangle]);
                }

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
    glDeleteProgram(shader);

    Quit(window, context);
    return 0;
}

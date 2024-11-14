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

void Quit(SDL_Window *window);
#define err(msg) Quit(window); ThrowError(msg);

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
Quit(SDL_Window *window)
{
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
            Quit(myWindow->window);
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

f32 cubeVertices[] = {
    // positions          // normals           // texture coords
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
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

glm::vec3 pointLightPositions[] = {
	glm::vec3( 0.7f,  0.2f,  2.0f),
	glm::vec3( 2.3f, -3.3f, -4.0f),
	glm::vec3(-4.0f,  2.0f, -12.0f),
	glm::vec3( 0.0f,  0.0f, -3.0f)
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
        log("[Texture] Failed to load texture %s", texturePath);
        stbi_image_free(data);
    }

    return texture;
}

f32 GetAlphaBlend(f32 frequency)
{
    f32 time = (f32)SDL_GetPerformanceCounter() / (f32)SDL_GetPerformanceFrequency();
    f32 alphaBlend = ((f32)sin(time * frequency) + 1.0f) / 2.0f;
    return alphaBlend;
}

f32 GetSecondsElapsed(u64 upTime, u64 frequency)
{
    u64 currTime = SDL_GetPerformanceCounter();
    f32 timeValue = (f32)((currTime - upTime) / (f32)frequency);
    return timeValue;
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
        lightShader = Shader(window, vertShaderPath, fragShaderPath);

        vertShaderPath = "../shaders/lightSource.vs";
        fragShaderPath = "../shaders/lightSource.fs";
        sourceLightShader = Shader(window, vertShaderPath, fragShaderPath);

        u32 nSquareVerts = ArrayCount(cubeVertices);
        u32 cube = GetVAOwithoutEBO(&vabo, cubeVertices, nSquareVerts);
        u32 lightSourceCube = GetVAOwithoutEBO(&vabo, cubeVertices, nSquareVerts);
        glEnable(GL_DEPTH_TEST);

        u32 container = GetTexture("../data/container.jpg", GL_MIRRORED_REPEAT);
        u32 container2 = GetTexture("../data/container2.png", GL_MIRRORED_REPEAT);
        u32 container2Specular = GetTexture("../data/container2_specular.png", GL_MIRRORED_REPEAT);
        u32 matrix = GetTexture("../data/matrix.jpg", GL_MIRRORED_REPEAT);
        u32 awesomeFace = GetTexture("../data/awesomeface.png", GL_MIRRORED_REPEAT);

        UseShader(lightShader);
        SetUniform(lightShader, "material.diffuse", 0);
        SetUniform(lightShader, "material.specular", 1);
        SetUniform(lightShader, "material.emission", 2);

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

                SetUniform(lightShader, "viewPos", debugCamera.position);

                glm::vec3 lightColor;
                lightColor.x = 0.4f;//1.0;
                lightColor.y = 0.7f;//1.0;
                lightColor.z = 0.1f;//1.0;

                // spotLight
                SetUniform(lightShader, "spotLight.ambient", 0.0f, 0.0f, 0.0f);
                SetUniform(lightShader, "spotLight.diffuse", 1.0f, 1.0f, 1.0f);
                SetUniform(lightShader, "spotLight.specular", 1.0f, 1.0f, 1.0f);
                SetUniform(lightShader, "spotLight.position", debugCamera.position);
                SetUniform(lightShader, "spotLight.direction", debugCamera.front);
                SetUniform(lightShader, "spotLight.constant", 1.0f);
                SetUniform(lightShader, "spotLight.linear", 0.09f);
                SetUniform(lightShader, "spotLight.quadratic", 0.032f);
                SetUniform(lightShader, "spotLight.cutOff", (f32)glm::cos(glm::radians(12.5)));
                SetUniform(lightShader, "spotLight.outerCutOff", (f32)glm::cos(glm::radians(15.0)));

                // TODO(marc): behaves weird around corners/diagonal movements.
                GetCameraDirection(&myWindow, newInput, &debugCamera, &lastMouseX, &lastMouseY);

                projection = glm::perspective(glm::radians(debugCamera.fov), f32(width / height), 0.1f, 100.0f);
                glm::mat4 view = GetViewMatrix(&debugCamera);
                SetUniform(lightShader, "projection", projection);
                SetUniform(lightShader, "view", view);

                // directional light
                SetUniform(lightShader, "dirLight.direction", -0.2f, -1.0f, -0.3f);
                SetUniform(lightShader, "dirLight.ambient", 0.05f, 0.05f, 0.05f);
                SetUniform(lightShader, "dirLight.diffuse", 0.4f, 0.4f, 0.4f);
                SetUniform(lightShader, "dirLight.specular", 0.5f, 0.5f, 0.5f);

                // point light 1-4
                char position[25];
                char ambient[25];
                char diffuse[25];
                char specular[25];
                char constant[25];
                char linear[25];
                char quadratic[25];

                for(u32 x = 0; x < ArrayCount(pointLightPositions); ++x)
                {
                    snprintf(position, sizeof(position), "pointLights[%d].position", x);
                    snprintf(ambient,  sizeof(ambient),  "pointLights[%d].ambient",  x);
                    snprintf(diffuse,  sizeof(diffuse),  "pointLights[%d].diffuse",  x);
                    snprintf(specular, sizeof(specular), "pointLights[%d].specular", x);
                    snprintf(constant, sizeof(constant), "pointLights[%d].constant", x);
                    snprintf(linear,   sizeof(linear),   "pointLights[%d].linear",   x);
                    snprintf(quadratic,sizeof(quadratic),"pointLights[%d].quadratic",x);

                    SetUniform(lightShader, position,  pointLightPositions[x]);
                    SetUniform(lightShader, ambient,   lightColor*0.1f);
                    SetUniform(lightShader, diffuse,   lightColor);
                    SetUniform(lightShader, specular,  1.0f, 1.0f, 1.0f);
                    SetUniform(lightShader, constant,  1.0f);
                    SetUniform(lightShader, linear,    0.09f);
                    SetUniform(lightShader, quadratic, 0.032f);
                }

                glm::mat4 model = glm::mat4(1.0f);
                SetUniform(lightShader, "model", model);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, container2);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, container2Specular);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, matrix);

                glBindVertexArray(vabo.VAO[cube]);
                for(u32 x = 0; x < 10; ++x)
                {
                    SetUniform(lightShader, "material.shininess", 32.0f);

                    model = glm::mat4(1.0f);
                    model = glm::translate(model, cubePositions[x]);
                    f32 angle = 20.0f * x;
                    model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
                    SetUniform(lightShader, "model", model);

                    glDrawArrays(GL_TRIANGLES, 0, vabo.count[cube]);
                }

                //lightPos.x = sin(timeValue)*3.0f+2.5f;
                //lightPos.y = -2.0;//sin(timeValue*2)*2;
                //lightPos.z = cos(timeValue)*3.0f+2.0f;

                UseShader(sourceLightShader);
                SetUniform(sourceLightShader, "view", view);
                SetUniform(sourceLightShader, "projection", projection);
                SetUniform(sourceLightShader, "color", lightColor);


                glBindVertexArray(vabo.VAO[lightSourceCube]);
                for(u32 x = 0; x < ArrayCount(pointLightPositions); ++x)
                {
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, pointLightPositions[x]);
                    model = glm::scale(model, glm::vec3(0.2f));

                    SetUniform(sourceLightShader, "model", model);

                    glDrawArrays(GL_TRIANGLES, 0, vabo.count[lightSourceCube]);
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
    glDeleteProgram(lightShader);
    glDeleteProgram(sourceLightShader);

    Quit(window);
    return 0;
}

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
    glGenBuffers(1, &vabo->VBO[vabo->nVBO]);

    glBindBuffer(GL_ARRAY_BUFFER, vabo->VBO[vabo->nVBO++]);
    glBufferData(GL_ARRAY_BUFFER, nVerts*sizeof(*vertices + 0), vertices, GL_STATIC_DRAW);

    glBindVertexArray(vabo->VAO[vabo->nVAO++]);

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

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

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

struct material
{
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    f32 shininess;
};
material materialValues[] = {
{ glm::vec3(0.0215,0.1745,0.0215) ,glm::vec3(0.07568,0.61424,0.07568),glm::vec3(0.633,0.727811,0.633) ,0.6}, //emerald
{ glm::vec3(0.135,0.2225,0.1575) ,glm::vec3(0.54,0.89,0.63) ,glm::vec3(0.316228,0.316228,0.316228),0.1}, //jade
{ glm::vec3(0.05375,0.05,0.06625) ,glm::vec3(0.18275,0.17,0.22525) ,glm::vec3(0.332741,0.328634,0.346435),0.3}, //obsidian
{ glm::vec3(0.25,0.20725,0.20725) ,glm::vec3(1,0.829,0.829) ,glm::vec3(0.296648,0.296648,0.296648),0.088}, //pearl
{ glm::vec3(0.1745,0.01175,0.01175),glm::vec3(0.61424,0.04136,0.04136),glm::vec3(0.727811,0.626959,0.626959),0.6}, //ruby
{ glm::vec3(0.1,0.18725,0.1745) ,glm::vec3(0.396,0.74151,0.69102) ,glm::vec3(0.297254,0.30829,0.306678) ,0.1}, //turquoise
{ glm::vec3(0.329412,0.223529,0.027451),glm::vec3(0.780392,0.568627,0.113725), glm::vec3(0.992157,0.941176,0.807843),0.21794872}, //brass
{ glm::vec3(0.2125,0.1275,0.054), glm::vec3(0.714,0.4284,0.18144) ,glm::vec3(0.393548,0.271906,0.166721), 0.2}, //bronze
{ glm::vec3(0.25,0.25,0.25) ,glm::vec3(0.4,0.4,0.4) ,glm::vec3(0.774597,0.774597,0.774597), 0.6}, //chrome
{ glm::vec3(0.19125,0.0735,0.0225) ,glm::vec3(0.7038,0.27048,0.0828) ,glm::vec3(0.256777,0.137622,0.086014),0.1}, //copper
{ glm::vec3(0.24725,0.1995,0.0745) ,glm::vec3(0.75164,0.60648,0.22648) ,glm::vec3(0.628281,0.555802,0.366065),0.4}, //gold
{ glm::vec3(0.19225,0.19225,0.19225),glm::vec3(0.50754,0.50754,0.50754) ,glm::vec3(0.508273,0.508273,0.508273),0.4}, //silver
{ glm::vec3(0.0,0.0,0.0) ,glm::vec3(0.01,0.01,0.01) ,glm::vec3(0.50,0.50,0.50), .25 }, //black plastic
{ glm::vec3(0.0,0.1,0.06) ,glm::vec3(0.0,0.50980392,0.50980392),glm::vec3(0.50196078,0.50196078,0.50196078), .25}, //cyan plastic
{ glm::vec3(0.0,0.0,0.0) ,glm::vec3(0.1 ,0.35,0.1) ,glm::vec3(0.45,0.55,0.45),.25 }, //green plastic
{ glm::vec3(0.0,0.0,0.0) ,glm::vec3(0.5 ,0.0,0.0) ,glm::vec3(0.7,0.6 ,0.6),.25 }, //red plastic
{ glm::vec3(0.0,0.0,0.0) ,glm::vec3(0.55 ,0.55,0.55) ,glm::vec3(0.70,0.70,0.70),.25 }, //white plastic
{ glm::vec3(0.0,0.0,0.0) ,glm::vec3(0.5 ,0.5,0.0) ,glm::vec3(0.60,0.60,0.50),.25 }, //yellow plastic
{ glm::vec3(0.02,0.02,0.02) ,glm::vec3(0.01,0.01,0.01) ,glm::vec3(0.4,0.4 ,0.4),.078125}, //black rubber
{ glm::vec3(0.0,0.05,0.05) ,glm::vec3(0.4 ,0.5,0.5) ,glm::vec3(0.04,0.7,0.7) ,.078125}, //cyan rubber
{ glm::vec3(0.0,0.05,0.0) ,glm::vec3(0.4 ,0.5,0.4) ,glm::vec3(0.04,0.7,0.04) ,.078125}, //green rubber
{ glm::vec3(0.05,0.0,0.0) ,glm::vec3(0.5 ,0.4,0.4) ,glm::vec3(0.7,0.04,0.04) ,.078125}, //red rubber
{ glm::vec3(0.05,0.05,0.05) ,glm::vec3(0.5,0.5,0.5) ,glm::vec3(0.7,0.7,0.7) ,.078125}, //white rubber
{ glm::vec3(0.05,0.05,0.0) ,glm::vec3(0.5,0.5,0.4) ,glm::vec3(0.7,0.7,0.04) ,.078125 } //yellow rubber
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

f32 GetSecondsElapsed(u64 upTime, u64 frequency)
{
    u64 currTime = SDL_GetPerformanceCounter();
    f32 timeValue = (f32)((currTime - upTime) / (f32)frequency);
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
        lightShader = Shader(window, vertShaderPath, fragShaderPath);

        vertShaderPath = "../shaders/lightSource.vs";
        fragShaderPath = "../shaders/lightSource.fs";
        sourceLightShader = Shader(window, vertShaderPath, fragShaderPath);
        glm::vec3 lightPos(1.2f, 0.0f, 2.0f);

        u32 nVerts1 = ArrayCount(middleTriangleVertices);
        u32 triangle1Verts = GetVAOwithoutEBO(&vabo, middleTriangleVertices, nVerts1);

        u32 nVerts2 = ArrayCount(triangleVertices1);
        u32 triangle2 = GetVAOwithoutEBO(&vabo, triangleVertices1, nVerts2);

        u32 nSquareVerts = ArrayCount(verticesWNormals);
        u32 cube = GetVAOwithoutEBO(&vabo, verticesWNormals, nSquareVerts);
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

                glm::vec3 lightColor;
                lightColor.x = 0.2;//sin(timeValue * 2.0f);
                lightColor.y = 0.2;//sin(timeValue * 0.7f);
                lightColor.z = 0.2;//sin(timeValue * 1.3f);

                glm::vec3 diffuseColor = lightColor   * glm::vec3(1.0f);
                glm::vec3 ambientColor = diffuseColor * glm::vec3(1.0f);

                UseShader(lightShader);
                SetUniform(lightShader, "light.ambient", ambientColor);
                SetUniform(lightShader, "light.diffuse", diffuseColor);
                SetUniform(lightShader, "light.specular", 1.0f, 1.0f, 1.0f);
                SetUniform(lightShader, "light.position", lightPos);

                SetUniform(lightShader, "viewPos", debugCamera.position);

                projection = glm::perspective(glm::radians(debugCamera.fov), f32(width / height), 0.1f, 100.0f);
                GetCameraDirection(&myWindow, newInput, &debugCamera, &lastMouseX, &lastMouseY);
                glm::mat4 view = GetViewMatrix(&debugCamera);

                SetUniform(lightShader, "view", view);
                SetUniform(lightShader, "projection", projection);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, container);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, awesomeFace);

                glBindVertexArray(vabo.VAO[cube]);
                glm::vec3 cubePos = cubePositions[0];
                for(u32 y = 0; y < 5; ++y)
                {
                    for(u32 x = 0; x < 5; ++x)
                    {
                        u32 matIdx = y*5 + x;
                        if(matIdx >= ArrayCount(materialValues)) matIdx = ArrayCount(materialValues)-1;
                        material cubeMaterial = materialValues[matIdx];

                        SetUniform(lightShader, "material.ambient", cubeMaterial.ambient);
                        SetUniform(lightShader, "material.diffuse", cubeMaterial.diffuse);
                        SetUniform(lightShader, "material.specular", cubeMaterial.specular);
                        SetUniform(lightShader, "material.shininess", cubeMaterial.shininess);

                        glm::mat4 model = glm::mat4(1.0f);
                        model = glm::translate(model, cubePositions[0]);
                        cubePositions[0].x += 1.2;
                        //f32 angle = 20.0f * x;
                        //model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
                        //if(x % 3 == 0 || x == 0 )
                        //{
                        //    model = glm::rotate(model, sin(timeValue)*5, glm::vec3(1.0f, 1.0f, 0.0f));
                        //}
                        SetUniform(lightShader, "model", model);

                        glDrawArrays(GL_TRIANGLES, 0, vabo.count[cube]);
                    }
                    cubePositions[0].x = cubePos.x;
                    cubePositions[0].y -= 1.2;
                }
                cubePositions[0] = cubePos;

                UseShader(sourceLightShader);
                SetUniform(sourceLightShader, "view", view);
                SetUniform(sourceLightShader, "projection", projection);
                SetUniform(sourceLightShader, "color", lightColor);

                glm::mat4 model = glm::mat4(1.0f);

                //lightPos.x = sin(timeValue)*3.0f+2.5f;
                //lightPos.y = sin(timeValue*2)*2;
                //lightPos.z = cos(timeValue)*3.0f+2.0f;

                lightPos.x = debugCamera.position.x + debugCamera.front.x;
                lightPos.y = debugCamera.position.y + debugCamera.front.y;
                lightPos.z = debugCamera.position.z + debugCamera.front.z;

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

    Quit(window);
    return 0;
}

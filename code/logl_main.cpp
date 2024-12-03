#ifndef LOGL_ROOT
#pragma once
#include "root.unity.h"
#endif

#include <SDL3/SDL.h>
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include <glad/glad.h>
#include <glad/glad.c>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf/cgltf.h>

#include <imgui/imgui.cpp>
#include <imgui/imgui_tables.cpp>
#include <imgui/imgui_widgets.cpp>
#include <imgui/imgui_draw.cpp>
#include <imgui/imgui_demo.cpp>
#include <imgui/backends/imgui_impl_opengl3.cpp>
#include <imgui/backends/imgui_impl_sdl3.cpp>

#include "Logl.h"

void Quit(SDL_Window *window);
#define err(msg) Quit(window); ThrowError(msg);

global_variable b32 globalQuit = false;
global_variable b32 firstMouse = true;
global_variable f32 alpha = 0;
global_variable f64 FPS = 0;

#include "logl_shader.cpp"
#include "logl_camera.cpp"
u32 GetTexture(char *texturePath, u32 wrappingMethod);
#include "logl_gltf.cpp"

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

    // TODO: turn these into arrays when needed
    u32 NUM_STRIPS;
    u32 NUM_VERTS_PER_STRIP;
    // TODO: get rid of this
    u32 rez;
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
            Log("ESCAPE PRESSED");
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

        if(key[SDL_SCANCODE_C])
        {
            ProcessKeyboardMessage(&keyboard->fovIn, isDown);
        }

        if(key[SDL_SCANCODE_V])
        {
            ProcessKeyboardMessage(&keyboard->fovOut, isDown);
        }

        if(key[SDL_SCANCODE_Z])
        {
            ProcessKeyboardMessage(&keyboard->zoomIn, isDown);
        }

        if(key[SDL_SCANCODE_X])
        {
            ProcessKeyboardMessage(&keyboard->zoomOut, isDown);
        }

        if(key[SDL_SCANCODE_F2])
        {
            ProcessKeyboardMessage(&keyboard->debug, isDown);
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
            Log("mouse focused!");
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

f32 globalGround[] = {
    // positions          // normals           // texture coords
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

};

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
        Log("[Texture] Failed to load texture %s", texturePath);
        stbi_image_free(data);
    }

    return texture;
}

u32 GenerateHeightMap(VABO *vabo, memory_arena *arena, char *heightMapPath, u32 *texture, u32 wrappingMethod)
{
    u32 vaoIndex = 0;

    i32 imageWidth, imageHeight, nrChannels;

    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrappingMethod);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrappingMethod);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    u8 *data = stbi_load(heightMapPath, &imageWidth, &imageHeight, &nrChannels, 0);
    stbi_set_flip_vertically_on_load(true);
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

    //u64 memorySize = Megabytes(4);
    u64 memorySize = Megabytes(164);
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
    u32 terrainShader = 0;
    if(!window)
    {
        ThrowError("Failed to initialized Window!");
    }
    else
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

        context = SDL_GL_CreateContext(window);
        if(!context)
        {
            err("Failed to initialize Context");
        }
        SDL_GL_SetSwapInterval(1); // Enable vsync

#ifndef LOGL_SLOW
        // enable debug callback
        glDebugMessageCallback(&DebugCallback, NULL);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

        SDL_SetWindowRelativeMouseMode(window, 1);

        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        {
            err("Failed to initialize GLAD");
        }

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        const char *glsl_version = "#version 410";
        // Setup Platform/Renderer backends
        ImGui_ImplSDL3_InitForOpenGL(window, context);
        ImGui_ImplOpenGL3_Init(glsl_version);

        bool show_demo_window = true;
        bool show_another_window = false;
        ImVec4 clear_color = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        ImVec4 shader_color = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);

        game_input input[2] {};
        game_input *newInput = &input[0];
        game_input *oldInput = &input[1];

        glViewport(0, 0, width, height);

        gltf_model backpack = {};
        LoadModel(&backpack, &arena, "../data/backpack/backpack.glb");

        char *vertShaderPath = "../shaders/light.vs";
        char *fragShaderPath = "../shaders/light.fs";
        lightShader = Shader(window, vertShaderPath, fragShaderPath);

        vertShaderPath = "../shaders/lightSource.vs";
        fragShaderPath = "../shaders/lightSource.fs";
        sourceLightShader = Shader(window, vertShaderPath, fragShaderPath);

        vertShaderPath = "../shaders/terrain.vs";
        fragShaderPath = "../shaders/terrain.fs";
        char *tcsShaderPath = "../shaders/terrain.tcs";
        char *tesShaderPath = "../shaders/terrain.tes";
        terrainShader = Shader(window, vertShaderPath, fragShaderPath, tcsShaderPath, tesShaderPath);

        u32 nSquareVerts = ArrayCount(cubeVertices);
        u32 cube = GetVAOwithoutEBO(&vabo, cubeVertices, nSquareVerts);
        u32 lightSourceCube = GetVAOwithoutEBO(&vabo, cubeVertices, nSquareVerts);

        u32 nground = ArrayCount(globalGround);
        u32 ground = GetVAOwithoutEBO(&vabo, globalGround, nground);

//        u32 terrain = GenerateHeightMapCPU(&vabo, &arena, "../data/iceland_heightmap.png");
        u32 terrainTexture = 0;
        u32 terrain = GenerateHeightMap(&vabo, &arena, "../data/iceland_heightmap.png", &terrainTexture, GL_MIRRORED_REPEAT);

        glEnable(GL_DEPTH_TEST);

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), f32(width / height), 0.1f, 100.0f);

        glm::vec3 camPos   = glm::vec3(0.0f, 0.0f,  3.0f);
        glm::vec3 camUp    = glm::vec3(0.0f, 1.0f,  0.0f);
        my_camera debugCamera = InitCamera(camPos, camUp);

        i32 nrAttributes;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
        Log("Maximum nr of vertex attributes supported: %d\n", nrAttributes);

        f32 scale = 64;
        f32 shift = 16;
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
        game_state gameState = PLAY;
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
                ImGui_ImplSDL3_ProcessEvent(&e);
                ProcessInput(&myWindow, newKeyboard, context, e);
                HandleEvent(&myWindow, e, debugCamera);
            }

            if(newKeyboard->debug.endedDown && gameState != DEBUG)
            {
                gameState = DEBUG;
            }
            else if(newKeyboard->debug.endedDown && gameState != PLAY)
            {
                gameState = PLAY;
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
            if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);

            // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
            {
                static float f = 0.0f;
                static int counter = 0;

                ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

                ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
                ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Another Window", &show_another_window);

                ImGui::SliderFloat("scale", &scale, 0.0f, 1000.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::SliderFloat("shift", &shift, 0.0f, 1000.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::ColorEdit3("shader color", (float*)&shader_color); // Edit 3 floats representing a color
                ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

                if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                    counter++;
                ImGui::SameLine();
                ImGui::Text("counter = %d", counter);

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
                ImGui::End();
            }

            // 3. Show another simple window.
            if (show_another_window)
            {
                ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
                ImGui::Text("Hello from another window!");
                if (ImGui::Button("Close Me"))
                    show_another_window = false;
                ImGui::End();
            }

            //Log("A BUTTON : %d", newKeyboard->moveLeft.endedDown);

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

                glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                f32 timeValue = GetSecondsElapsed(upTime, frequency);
                f32 ab = GetAlphaBlend(1.0f);

                UseShader(lightShader);

                SetUniform(lightShader, "viewPos", debugCamera.position);

                glm::vec3 lightColor;
                lightColor.x = 1.0;//0.4f;
                lightColor.y = 1.0;//0.7f;
                lightColor.z = 1.0;//0.1f;

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

                switch(gameState)
                {
                    case DEBUG:
                    {
                        SDL_SetWindowRelativeMouseMode(window, 0);
                    } break;

                    case PLAY:
                    {
                        SDL_SetWindowRelativeMouseMode(window, 1);
                        // TODO(marc): behaves weird around corners/diagonal movements.
                        GetCameraDirection(&myWindow, newInput, &debugCamera, &lastMouseX, &lastMouseY);
                    } break;
                }
                ProcessCameraInputs(&debugCamera, newKeyboard, newInput->dtForFrame);
                projection = glm::perspective(glm::radians(debugCamera.fov), f32(width / height), 0.1f, 1000.0f);
                glm::mat4 view = MYDEBUGGetViewMatrix(&debugCamera);
                SetUniform(lightShader, "projection", projection);
                SetUniform(lightShader, "view", view);

                // directional light
                SetUniform(lightShader, "dirLight.direction", -0.2f, -1.0f, -0.3f);
                SetUniform(lightShader, "dirLight.ambient", 0.05f, 0.05f, 0.05f);
                SetUniform(lightShader, "dirLight.diffuse", 0.4f, 0.4f, 0.4f);
                SetUniform(lightShader, "dirLight.specular", 0.5f, 0.5f, 0.5f);

                // point light 1-4
                char position[30];
                char ambient[30];
                char diffuse[30];
                char specular[30];
                char constant[30];
                char linear[30];
                char quadratic[30];

                for(u32 x = 0; x < ArrayCount(pointLightPositions); ++x)
                {
                    snprintf(position, sizeof(position), "pointLights[%d].position", x);
                    snprintf(ambient,  sizeof(ambient),  "pointLights[%d].ambient",  x);
                    snprintf(diffuse,  sizeof(diffuse),  "pointLights[%d].diffuse",  x);
                    snprintf(specular, sizeof(specular), "pointLights[%d].specular", x);
                    snprintf(constant, sizeof(constant), "pointLights[%d].constant", x);
                    snprintf(linear,   sizeof(linear),   "pointLights[%d].linear",   x);
                    snprintf(quadratic,sizeof(quadratic),"pointLights[%d].quadratic",x);

                    SetUniform(lightShader, position, glm::vec3(0, sin(timeValue)*2, cos(timeValue)*2));
                    SetUniform(lightShader, ambient,   lightColor*0.1f);
                    SetUniform(lightShader, diffuse,   lightColor);
                    SetUniform(lightShader, specular,  0.8f, 0.8f, 0.8f);
                    SetUniform(lightShader, constant,  1.0f);
                    SetUniform(lightShader, linear,    0.09f);
                    SetUniform(lightShader, quadratic, 0.032f);
                }
                SetUniform(lightShader, "material.shininess", (f32)32);

                DrawModel(&backpack, lightShader);

                UseShader(sourceLightShader);
                SetUniform(sourceLightShader, "view", view);
                SetUniform(sourceLightShader, "projection", projection);
                SetUniform(sourceLightShader, "color", lightColor);

                glm::mat4 model = glm::mat4(1.0f);
                glBindVertexArray(vabo.VAO[lightSourceCube]);
                for(u32 x = 0; x < ArrayCount(pointLightPositions); ++x)
                {
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, glm::vec3(0, sin(timeValue)*2, cos(timeValue)*2));
                    model = glm::scale(model, glm::vec3(0.2f));

                    SetUniform(sourceLightShader, "model", model);

                    glDrawArrays(GL_TRIANGLES, 0, vabo.count[lightSourceCube]);
                }

#if 0
                glBindVertexArray(vabo.VAO[ground]);

                lightColor.x = 1.4f;
                lightColor.y = 1.7f;
                lightColor.z = 1.1f;

                SetUniform(sourceLightShader, "color", lightColor);

                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(0, -2, 0));
                model = glm::scale(model, glm::vec3(20.0f));

                SetUniform(sourceLightShader, "model", model);
                glDrawArrays(GL_TRIANGLES, 0, vabo.count[ground]);
#endif

                UseShader(terrainShader);

                SetUniform(terrainShader, "heightMap", 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, terrainTexture);

                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(0, -10, 0));
                model = glm::scale(model, glm::vec3(0.2));

                SetUniform(terrainShader, "view", view);
                SetUniform(terrainShader, "projection", projection);
                SetUniform(terrainShader, "model", model);

                SetUniform(terrainShader, "scale", scale);
                SetUniform(terrainShader, "shift", shift);

                SetUniform(terrainShader, "shaderR", shader_color.x*shader_color.w);
                SetUniform(terrainShader, "shaderG", shader_color.y*shader_color.w);
                SetUniform(terrainShader, "shaderB", shader_color.z*shader_color.w);

                glBindVertexArray(vabo.VAO[terrain]);
                glDrawArrays(GL_PATCHES, 0, 20*vabo.rez*vabo.rez);
#if 0
                for(u32 strip = 0; strip < vabo.NUM_STRIPS; ++strip)
                {
                    glDrawElements(GL_TRIANGLE_STRIP,
                                   vabo.NUM_VERTS_PER_STRIP,
                                   GL_UNSIGNED_INT,
                                   (void*)(sizeof(u32) *
                                    vabo.NUM_VERTS_PER_STRIP *
                                    strip));
                }
#endif


                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                SDL_GL_SwapWindow(window);

                // TODO: track fps
                u64 endCycleCount = SDL_GetPerformanceCounter();
                cyclesElapsed = endCycleCount - lastCycleCount;
                lastCycleCount = endCycleCount;

                MCPF = (f64)cyclesElapsed / (f64)frequency;
#if 1
                FPS = 1.0f/MCPF;

                //Log("%.02ff/s,  %.02fmc/f\n", FPS, MCPF);
#endif
            }
        }

    }

    // TODO: de-allocate resources
    glDeleteVertexArrays(vabo.nVAO, vabo.VAO);
    glDeleteBuffers(vabo.nVBO, vabo.VBO);
    glDeleteProgram(lightShader);
    glDeleteProgram(sourceLightShader);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    Quit(window);
    return 0;
}

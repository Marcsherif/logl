#ifndef LOGL_ROOT
#pragma once
#include "root.unity.h"
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
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

// networking for live link
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "Ws2_32.lib")

#include "logl.h"

void Quit(SDL_Window *sdl_window);
#define err(msg) Quit(sdl_window); ThrowError(msg);

global_variable b32 globalQuit = false;
global_variable b32 firstMouse = true;
global_variable f32 alpha = 0;
global_variable f64 FPS = 0;

#define WINDOW_WIDTH 920
#define WINDOW_HEIGHT 780

f32 GetSecondsElapsed(u64 upTime, u64 frequency);
f32 GetAlphaBlend(f32 frequency);
#include "logl.cpp"

void
Quit(SDL_Window *window)
{
    glDeleteVertexArrays(vabo->nVAO, vabo->VAO);
    glDeleteBuffers(vabo->nVBO, vabo->VBO);
    glDeleteProgram(lightShader);
    glDeleteProgram(sourceLightShader);

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
HandleContinuousInput(game_controller_input *keyboard)
{
    const bool *state = SDL_GetKeyboardState(NULL);

    ProcessKeyboardMessage(&keyboard->moveForward,  state[SDL_SCANCODE_W]);
    ProcessKeyboardMessage(&keyboard->moveBackward, state[SDL_SCANCODE_S]);
    ProcessKeyboardMessage(&keyboard->moveLeft,     state[SDL_SCANCODE_A]);
    ProcessKeyboardMessage(&keyboard->moveRight,    state[SDL_SCANCODE_D]);
    ProcessKeyboardMessage(&keyboard->speedUp,      state[SDL_SCANCODE_LSHIFT]);

    ProcessKeyboardMessage(&keyboard->fovIn,        state[SDL_SCANCODE_C]);
    ProcessKeyboardMessage(&keyboard->fovOut,       state[SDL_SCANCODE_V]);

    ProcessKeyboardMessage(&keyboard->zoomIn, state[SDL_SCANCODE_Z]);
    ProcessKeyboardMessage(&keyboard->zoomOut, state[SDL_SCANCODE_X]);
}

internal void
ProcessInput(my_window *myWindow, game_controller_input *keyboard, SDL_GLContext context, SDL_Event e)
{
    if(e.type == SDL_EVENT_QUIT) globalQuit = true;

    if(e.type == SDL_EVENT_KEY_DOWN)
    {
        switch(e.key.scancode)
        {
            case SDL_SCANCODE_RETURN:
            {
                //if (e.key.mod & SDL_KMOD_ALT)
                {
                    myWindow->fullScreen = !myWindow->fullScreen;
                    SDL_SetWindowFullscreen(myWindow->sdl_window, (bool)myWindow->fullScreen);
                    myWindow->minimized = false;
                }
            } break;

            case SDL_SCANCODE_ESCAPE:
            {
                Log("ESCAPE PRESSED");
                globalQuit = true;
                Quit(myWindow->sdl_window);
            } break;
        }
    }

    if(e.type == SDL_EVENT_KEY_UP)
    {
        if(e.key.scancode == SDL_SCANCODE_F2)
            ProcessKeyboardMessage(&keyboard->debug, true);
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
            break;

        case SDL_EVENT_WINDOW_EXPOSED:
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
        // SDL_snprintf(fovText, 30, "  Fov: %.02f  FPS: %.02ff/s", camera.fov, FPS);
        SDL_snprintf(fovText, 30, "  Fov: %.02f", camera.fov);
        SDL_strlcat(caption, fovText, bufSize);
        SDL_SetWindowTitle(myWindow->sdl_window, caption);
    }
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

    SDL_Window *sdl_window = SDL_CreateWindow("Hello, world!", WINDOW_WIDTH, WINDOW_HEIGHT,
                                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    SDL_SetWindowPosition(sdl_window, WINDOW_WIDTH +80 , 40);
    SDL_GLContext context = 0;

    //u64 memorySize = Megabytes(4);
    game_memory memory = {};
    memory.permanent_storage_size = megabytes(164);
    u64 memory_size = memory.permanent_storage_size;
    memory.permanent_storage = (void *)VirtualAlloc(0, memory_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    if(!sdl_window)
    {
        ThrowError("Failed to initialized Window!");
    }
    else
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

        context = SDL_GL_CreateContext(sdl_window);
        if(!context)
        {
            err("Failed to initialize Context");
        }
        SDL_GL_MakeCurrent(sdl_window, context);
        SDL_GL_SetSwapInterval(1); // Enable vsync

#ifndef LOGL_SLOW
        // enable debug callback
        glDebugMessageCallback(&DebugCallback, NULL);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

        //SDL_SetWindowRelativeMouseMode(sdl_window, 1); // hide or show mouse

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
        ImGui_ImplSDL3_InitForOpenGL(sdl_window, context);
        ImGui_ImplOpenGL3_Init(glsl_version);

        bool show_demo_window = true;
        bool show_another_window = false;
        gui_vars gui;
        gui.clear_color = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        gui.shader_color = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        gui.scale = 64;
        gui.shift = 16;

        game_input input[2] {};
        game_input *new_input = &input[0];
        game_input *old_input = &input[1];

        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

        i32 nrAttributes;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
        Log("Maximum nr of vertex attributes supported: %d\n", nrAttributes);

        my_window window;
        window.sdl_window = sdl_window;
        SDL_Event e;
        new_input->uptime = SDL_GetPerformanceCounter();
        new_input->frequency = SDL_GetPerformanceFrequency();
        u64 lastCycleCount = new_input->uptime;
        u64 cyclesElapsed = lastCycleCount;
        f64 MCPF = 0;
        new_input->last_mouse_x = WINDOW_WIDTH/2;
        new_input->last_mouse_y = WINDOW_HEIGHT/2;
        i32 movement = 0;
        while(!globalQuit)
        {
            new_input->dt = (f32)MCPF;

            game_controller_input *oldKeyboard = GetController(old_input, 0);
            game_controller_input *newKeyboard = GetController(new_input, 0);
            *newKeyboard = {};
            newKeyboard->isConnected = true;
            for(i32 buttonIndex = 0;
                buttonIndex < ArrayCount(newKeyboard->buttons);
                ++buttonIndex)
            {
                newKeyboard->buttons[buttonIndex].endedDown =
                    oldKeyboard->buttons[buttonIndex].endedDown;
            }

            Game_State *game_state = (Game_State *)memory.permanent_storage;

            while(SDL_PollEvent(&e))
            {
                ImGui_ImplSDL3_ProcessEvent(&e);
                ProcessInput(&window, newKeyboard, context, e);
                HandleEvent(&window, e, game_state->debug_camera);
            }

            HandleContinuousInput(newKeyboard);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
            io.FontGlobalScale = 2;

            // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).AA
            //if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

            // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
            {
                static float f = 0.0f;
                static int counter = 0;

                ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

                ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
                ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Another Window", &show_another_window);
                ImGui::Text("cam front = %f %f %f", game_state->debug_camera.front.x, game_state->debug_camera.front.y, game_state->debug_camera.front.z);
                ImGui::Text("cam vel = %f %f %f", game_state->debug_camera.vel.x, game_state->debug_camera.vel.y, game_state->debug_camera.vel.z);
                ImGui::Text("MOUSE: %d %d", new_input->mouseButtons[0].endedDown, new_input->mouseButtons[1].endedDown);

                ImGui::SliderFloat("scale", &gui.scale, 0.0f, 1000.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::SliderFloat("shift", &gui.shift, 0.0f, 1000.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::ColorEdit3("shader color", (float*)&gui.shader_color); // Edit 3 floats representing a color
                ImGui::ColorEdit3("clear color", (float*)&gui.clear_color); // Edit 3 floats representing a color

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
            u32 mouseState = SDL_GetMouseState(&mousePosX, &mousePosY);
            new_input->mouseX = mousePosX;
            new_input->mouseY = mousePosY;
            new_input->mouseZ = 0; // TODO(marc): Support mousewheel?

            ProcessKeyboardMessage(&new_input->mouseButtons[0], (mouseState & SDL_BUTTON_LMASK));
            ProcessKeyboardMessage(&new_input->mouseButtons[1], (mouseState & SDL_BUTTON_RMASK));

            if(!window.minimized)
            {
                game_update_and_render(&window, &memory, new_input, &gui);

                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                SDL_GL_SwapWindow(sdl_window);

                // TODO: track fps
                u64 endCycleCount = SDL_GetPerformanceCounter();
                cyclesElapsed = endCycleCount - lastCycleCount;
                lastCycleCount = endCycleCount;

                MCPF = (f64)cyclesElapsed / (f64)new_input->frequency;
#if 1
                FPS = 1.0f/MCPF;

                //Log("%.02ff/s,  %.02fmc/f\n", FPS, MCPF);
#endif
            }
        }

    }

    // TODO: de-allocate resources
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    Quit(sdl_window);
    return 0;
}

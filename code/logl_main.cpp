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
#include <new>
#include "logl_runtime.cpp"

void quit(SDL_Window *sdl_window);
#define err(msg) quit(sdl_window); throw_error(msg);

global_variable b32 global_quit = false;
global_variable b32 first_mouse = true;

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 800

f32 get_seconds_elapsed(u64 up_time, u64 frequency);
#include "logl_geometry_data.h"
#include "logl_opengl.cpp"
#include "logl_camera.cpp"
#include "logl_gltf.cpp"
#include "logl_livelink.cpp"
#include "logl_animation.cpp"
#include "logl_workbench.cpp"
#include "logl.cpp"

void
quit(SDL_Window *window)
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}

internal void
process_keyboard_message(Game_Button_State *new_state, b32 is_down)
{
    if(new_state->ended_down != is_down)
    {
        new_state->ended_down = is_down;
        ++new_state->half_transition_count;
    }
}

internal void
handle_continuous_input(Game_Controller_Input *keyboard)
{
    const bool *state = SDL_GetKeyboardState(NULL);

    process_keyboard_message(&keyboard->move_forward,  state[SDL_SCANCODE_W]);
    process_keyboard_message(&keyboard->move_backward, state[SDL_SCANCODE_S]);
    process_keyboard_message(&keyboard->move_left,     state[SDL_SCANCODE_A]);
    process_keyboard_message(&keyboard->move_right,    state[SDL_SCANCODE_D]);
    process_keyboard_message(&keyboard->speed_up,      state[SDL_SCANCODE_LSHIFT]);

    process_keyboard_message(&keyboard->fov_in,        state[SDL_SCANCODE_C]);
    process_keyboard_message(&keyboard->fov_out,       state[SDL_SCANCODE_V]);

    process_keyboard_message(&keyboard->zoom_in, state[SDL_SCANCODE_Z]);
    process_keyboard_message(&keyboard->zoom_out, state[SDL_SCANCODE_X]);
}

internal void
process_input(Window *my_window, Game_Controller_Input *keyboard, SDL_GLContext context, SDL_Event e)
{
    if(e.type == SDL_EVENT_QUIT) global_quit = true;

    if(e.type == SDL_EVENT_KEY_DOWN)
    {
        switch(e.key.scancode)
        {
            case SDL_SCANCODE_RETURN:
            {
                //if (e.key.mod & SDL_KMOD_ALT)
                {
                    my_window->full_screen = !my_window->full_screen;
                    SDL_SetWindowFullscreen(my_window->sdl_window, (bool)my_window->full_screen);
                    my_window->minimized = false;
                }
            } break;

            case SDL_SCANCODE_ESCAPE:
            {
                log_message("ESCAPE PRESSED");
                global_quit = true;
            } break;

            default:
            break;
        }
    }

    if(e.type == SDL_EVENT_KEY_UP)
    {
        if(e.key.scancode == SDL_SCANCODE_F2)
            process_keyboard_message(&keyboard->debug, true);
    }
}

void handle_event(Window *my_window, SDL_Event e, Camera camera)
{
    switch(e.type)
    {
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            my_window->width = e.window.data1;
            my_window->height = e.window.data2;
            framebuffer_size_callback(my_window->width, my_window->height);
            break;

        case SDL_EVENT_WINDOW_EXPOSED:
            break;

        case SDL_EVENT_WINDOW_MOUSE_ENTER:
            my_window->mouse_focus = true;
            my_window->update_caption = true;
            log_message("mouse focused!");
            break;

        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
            my_window->mouse_focus = false;
            my_window->update_caption = true;
            break;

        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            my_window->keyboard_focus = true;
            my_window->update_caption = true;
            break;

        case SDL_EVENT_WINDOW_FOCUS_LOST:
            my_window->keyboard_focus = false;
            my_window->update_caption = true;
            break;

        case SDL_EVENT_WINDOW_MINIMIZED:
            my_window->minimized = true;
            break;

        case SDL_EVENT_WINDOW_MAXIMIZED:
            my_window->minimized = false;
            break;

        case SDL_EVENT_WINDOW_RESTORED:
            my_window->minimized = false;
            break;
    }
    if(my_window->update_caption)
    {
        SDL_SetWindowTitle(my_window->sdl_window, "LOGL | Rendering & Animation Workbench");
        my_window->update_caption = false;
    }
}

f32 get_seconds_elapsed(u64 up_time, u64 frequency)
{
    u64 curr_time = SDL_GetPerformanceCounter();
    f32 time_value = (f32)((curr_time - up_time) / (f32)frequency);
    return time_value;
}

int main()
{
    load_runtime();

    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        throw_error("Failed to initialized SDL3!");
    }

    SDL_Window *sdl_window = SDL_CreateWindow("LOGL | Rendering & Animation Workbench", WINDOW_WIDTH, WINDOW_HEIGHT,
                                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    SDL_SetWindowPosition(sdl_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_GLContext context = 0;

    //u64 memorySize = Megabytes(4);
    Game_Memory memory = {};
    memory.permanent_storage_size = megabytes(164);
    u64 memory_size = memory.permanent_storage_size;
    memory.permanent_storage = (void *)VirtualAlloc(0, memory_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    if(!memory.permanent_storage)
    {
        throw_error("Failed to allocate scene memory");
    }
    // The character owns an unordered_map, so its constructor must run.
    Game_State *game_state = new(memory.permanent_storage) Game_State{};

    if(!sdl_window)
    {
        throw_error("Failed to initialized Window!");
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
        glDebugMessageCallback(&debug_callback, NULL);
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
        ImGuiStyle &style = ImGui::GetStyle();
        style.WindowPadding = ImVec2(16, 16);
        style.FramePadding = ImVec2(8, 6);
        style.ItemSpacing = ImVec2(8, 10);
        style.FrameRounding = 4;
        style.GrabRounding = 4;
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.38f, 0.78f, 0.93f, 1);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.65f, 0.82f, 1);

        const char *glsl_version = "#version 410";
        // Setup Platform/Renderer backends
        ImGui_ImplSDL3_InitForOpenGL(sdl_window, context);
        ImGui_ImplOpenGL3_Init(glsl_version);

        Game_Input input[2] {};
        Game_Input *new_input = &input[0];
        Game_Input *old_input = &input[1];

        Workbench_Settings *dbg_gui = &game_state->dbg_gui;
        dbg_gui->clear_color = glm::vec4(0.055f, 0.07f, 0.09f, 1.00f);
        dbg_gui->shader_color = glm::vec4(0.3f, 0.48f, 0.42f, 1.00f);
        dbg_gui->scale = 64;
        dbg_gui->shift = 16;

        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

        i32 nr_attributes;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nr_attributes);
        log_message("Maximum nr of vertex attributes supported: %d\n", nr_attributes);

        Window window = {};
        window.sdl_window = sdl_window;
        SDL_GetWindowSizeInPixels(sdl_window, &window.width, &window.height);
        SDL_Event e;
        new_input->uptime = SDL_GetPerformanceCounter();
        new_input->frequency = SDL_GetPerformanceFrequency();
        u64 last_cycle_count = new_input->uptime;
        u64 cycles_elapsed = last_cycle_count;
        f64 seconds_per_frame = 0;
        new_input->last_mouse_x = WINDOW_WIDTH/2;
        new_input->last_mouse_y = WINDOW_HEIGHT/2;
        i32 movement = 0;
        while(!global_quit)
        {
            new_input->dt = (f32)seconds_per_frame;

            Game_Controller_Input *old_keyboard = get_controller(old_input, 0);
            Game_Controller_Input *new_keyboard = get_controller(new_input, 0);
            *new_keyboard = {};
            new_keyboard->is_connected = true;
            for(i32 button_index = 0;
                button_index < array_count(new_keyboard->buttons);
                ++button_index)
            {
                new_keyboard->buttons[button_index].ended_down =
                    old_keyboard->buttons[button_index].ended_down;
            }

            while(SDL_PollEvent(&e))
            {
                ImGui_ImplSDL3_ProcessEvent(&e);
                process_input(&window, new_keyboard, context, e);
                handle_event(&window, e, game_state->debug_camera);
            }

            handle_continuous_input(new_keyboard);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
            io.FontGlobalScale = 1;

            if (memory.is_initialized)
                draw_workbench(game_state);

            f32 mouse_pos_x, mouse_pos_y;
            u32 mouse_state = SDL_GetMouseState(&mouse_pos_x, &mouse_pos_y);
            new_input->mouse_x = mouse_pos_x;
            new_input->mouse_y = mouse_pos_y;
            new_input->mouse_z = 0; // TODO(marc): Support mousewheel?

            process_keyboard_message(&new_input->mouse_buttons[0], (mouse_state & SDL_BUTTON_LMASK));
            process_keyboard_message(&new_input->mouse_buttons[1], (mouse_state & SDL_BUTTON_RMASK));

            if(!window.minimized)
            {
                game_update_and_render(&window, &memory, new_input);

                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                SDL_GL_SwapWindow(sdl_window);

                u64 end_cycle_count = SDL_GetPerformanceCounter();
                cycles_elapsed = end_cycle_count - last_cycle_count;
                last_cycle_count = end_cycle_count;

                seconds_per_frame = (f64)cycles_elapsed / (f64)new_input->frequency;
            }
        }

    }

    if(memory.is_initialized) shutdown_scene(game_state);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    game_state->~Game_State();
    VirtualFree(memory.permanent_storage, 0, MEM_RELEASE);
    SDL_GL_DestroyContext(context);
    quit(sdl_window);
    return 0;
}

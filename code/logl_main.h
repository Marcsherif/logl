#if !defined(SDL_MAIN_H)

struct my_window
{
    SDL_Window *sdl_window;
    b32 updateCaption = false;
    i32 width;
    i32 height;
    b32 fullScreen = false;
    b32 minimized = false;
    b32 mouseFocus;
    b32 keyboardFocus;
};

struct gui_vars
{
    ImVec4 clear_color;
    ImVec4 shader_color;
    f32 scale;
    f32 shift;
};

#define SDL_MAIN_H
#endif

#if !defined(SDL_MAIN_H)

struct my_window
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

#define SDL_MAIN_H
#endif

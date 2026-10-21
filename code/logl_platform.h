#if !defined(LOGL_PLATFORM_H)

#include <stdint.h>
#include <stddef.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef i32 b32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t Memory_Index;

typedef float f32;
typedef double f64;

#define internal static
#define local_persist static
#define global_variable static

#ifdef LOGL_SLOW
#define assert_slow(cond) do { if (!(cond)) __debugbreak(); } while (0)
#else
#define assert_slow(cond)
#endif

#define invalid_code_path assert_slow(!"InvalidCodePath");

#define kilobytes(value) ((value)*1024LL)
#define megabytes(value) (kilobytes(value)*1024LL)
#define gigabytes(value) (megabytes(value)*1024LL)
#define terabytes(value) (gigabytes(value)*1024LL)

#define array_count(array) (sizeof(array) / sizeof((array)[0]))

#define swap_values(a, b, t) {t temp = a; a = b; b = temp;}

inline u32
safe_truncate_u64(u64 value)
{
    assert_slow(value <= 0xFFFFFFFF);
    u32 result = (u32)value;
    return(result);
}

void throw_error(const char *message)
{
    MessageBoxA(nullptr, message, "LOGL ERROR", MB_OK | MB_ICONERROR);
    ExitProcess(0);
}

void log_message(const char* msg, ...)
{
    char buf[4096];
    va_list vl;

    va_start(vl, msg);
    vsnprintf(buf, sizeof(buf), msg, vl);
    va_end(vl);
    printf("%s\n", buf);
}

#ifdef LOGL_SLOW
static void APIENTRY debug_callback(
    GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* message, const void* user)
{
    OutputDebugStringA(message);
    OutputDebugStringA("\n");
    if (severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM)
    {
        if (IsDebuggerPresent())
        {
            assert_slow(!"OpenGL error - check the callstack in debugger");
        }
        throw_error("OpenGL API usage error! Use debugger to examine call stack!");
    }
}
#endif

/*
 NOTE(marc): Services that the game provides to the platform layer.
 (this may expand in the future - sound on seperate thread, etc.)
*/

struct SDL_Window;

struct Window
{
    SDL_Window *sdl_window;
    b32 update_caption = false;
    i32 width;
    i32 height;
    b32 full_screen = false;
    b32 minimized = false;
    b32 mouse_focus;
    b32 keyboard_focus;
};

struct Game_Button_State
{
    int half_transition_count;
    b32 ended_down;
};

struct Game_Controller_Input
{
    b32 is_connected;
    b32 is_analog;
    f32 stick_average_x;
    f32 stick_average_y;

    union
    {
        Game_Button_State buttons[10];
        struct
        {
            Game_Button_State move_left;
            Game_Button_State move_right;
            Game_Button_State move_forward;
            Game_Button_State move_backward;

            Game_Button_State speed_up;

            Game_Button_State fov_in;
            Game_Button_State fov_out;

            Game_Button_State zoom_in;
            Game_Button_State zoom_out;

            Game_Button_State debug;

            //

            Game_Button_State terminator;
        };
    };
};

struct Workbench_Settings
{
    glm::vec4 clear_color;
    glm::vec4 shader_color;
    f32 scale;
    f32 shift;
    bool show_character = true;
    bool show_terrain = false;
    bool show_light_markers = false;
    bool animate_lights = false;
    bool wireframe = false;
    bool reset_camera = false;
    bool show_skeleton = true;
    bool show_joint_names = false;
    bool show_weights = false;
    i32 selected_joint = -1;
};

struct Game_Input
{
    // Mouse for debugging
    Game_Button_State mouse_buttons[5];
    f32 mouse_x, mouse_y, mouse_z;

    f32 dt;
    u64 uptime;
    u64 frequency;
    f32 last_mouse_x;
    f32 last_mouse_y;

    Game_Controller_Input controllers[5];
};

inline Game_Controller_Input *get_controller(Game_Input *input, int unsigned controller_index)
{
    assert_slow(controller_index < array_count(input->controllers));
    Game_Controller_Input *result = &input->controllers[controller_index];
    return(result);
}

struct Game_Memory
{
    b32 is_initialized;

    u64 permanent_storage_size;
    void *permanent_storage; // NOTE(marc): Required to be cleared to be zero at startup

    u64 transient_storage_size;
    void *transient_storage; // NOTE(marc): Required to be cleared to be zero at startup

//    debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
//    debug_platform_free_entire_file *DEBUGPlatformFreeEntireFile;
//    debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
};

#define LOGL_PLATFORM_H
#endif

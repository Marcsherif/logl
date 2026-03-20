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

typedef size_t memory_index;

typedef float f32;
typedef double f64;

#define internal static
#define local_persist static
#define global_variable static

#ifdef LOGL_SLOW
#define Assert(cond) do { if (!(cond)) __debugbreak(); } while (0)
#else
#define Assert(cond)
#endif

#define InvalidCodePath Assert(!"InvalidCodePath");

#define kilobytes(Value) ((Value)*1024LL)
#define megabytes(Value) (kilobytes(Value)*1024LL)
#define gigabytes(Value) (megabytes(Value)*1024LL)
#define terabytes(Value) (gigabytes(Value)*1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Swap(a, b, t) {t temp = a; a = b; b = temp;}

inline u32
SafeTruncateUInt64(u64 Value)
{
    Assert(Value <= 0xFFFFFFFF);
    u32 Result = (u32)Value;
    return(Result);
}

void ThrowError(const char *message)
{
    MessageBoxA(nullptr, message, "LOGL ERROR", MB_OK | MB_ICONERROR);
    ExitProcess(0);
}

void Log(const char* msg, ...)
{
    char buf[4096];
    va_list vl;

    va_start(vl, msg);
    vsnprintf(buf, sizeof(buf), msg, vl);
    va_end(vl);
    printf("%s\n", buf);
}

#ifdef LOGL_SLOW
static void APIENTRY DebugCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* message, const void* user)
{
    OutputDebugStringA(message);
    OutputDebugStringA("\n");
    if (severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM)
    {
        if (IsDebuggerPresent())
        {
            Assert(!"OpenGL error - check the callstack in debugger");
        }
        ThrowError("OpenGL API usage error! Use debugger to examine call stack!");
    }
}
#endif

/*
 NOTE(marc): Services that the game provides to the platform layer.
 (this may expand in the future - sound on seperate thread, etc.)
*/

struct game_button_state
{
    int halfTransitionCount;
    b32 endedDown;
};

struct game_controller_input
{
    b32 isConnected;
    b32 isAnalog;
    f32 stickAverageX;
    f32 stickAverageY;

    union
    {
        game_button_state buttons[10];
        struct
        {
            game_button_state moveLeft;
            game_button_state moveRight;
            game_button_state moveForward;
            game_button_state moveBackward;

            game_button_state speedUp;

            game_button_state fovIn;
            game_button_state fovOut;

            game_button_state zoomIn;
            game_button_state zoomOut;

            game_button_state debug;

            //

            game_button_state terminator;
        };
    };
};

struct game_input
{
    // Mouse for debugging
    game_button_state mouseButtons[5];
    f32 mouseX, mouseY, mouseZ;

    f32 dt;
    u64 uptime;
    u64 frequency;
    f32 last_mouse_x;
    f32 last_mouse_y;

    game_controller_input controllers[5];
};

inline game_controller_input *GetController(game_input *input, int unsigned controller_index)
{
    Assert(controller_index < ArrayCount(input->controllers));
    game_controller_input *result = &input->controllers[controller_index];
    return(result);
}

struct game_memory
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

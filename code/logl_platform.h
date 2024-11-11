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
#define Assert(cond) if(!(cond)) {*(int *)0 = 0;}
#else
#define Assert(cond)
#endif

#define InvalidCodePath Assert(!"InvalidCodePath");

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

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
    __debugbreak();
}

void log(const char* msg, ...)
{
    char buf[4096];
    va_list vl;

    va_start(vl, msg);
    vsnprintf(buf, sizeof(buf), msg, vl);
    va_end(vl);
    printf("%s\n", buf);
}

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
        game_button_state buttons[6];
        struct
        {
            game_button_state moveLeft;
            game_button_state moveRight;
            game_button_state moveForward;
            game_button_state moveBackward;

            game_button_state zoomin;
            game_button_state zoomout;

            //

            game_button_state Terminator;
        };
    };
};

struct game_input
{
    // Mouse for debugging
    game_button_state mouseButtons[5];
    f32 mouseX, mouseY, mouseZ;

    f32 dtForFrame;

    game_controller_input controllers[5];
};

inline game_controller_input *GetController(game_input *input, int unsigned controllerIndex)
{
    Assert(controllerIndex < ArrayCount(input->controllers));
    game_controller_input *result = &input->controllers[controllerIndex];
    return(result);
}

#define LOGL_PLATFORM_H
#endif

#if !defined(LOGL_H)

#include "logl_platform.h"

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

struct memory_arena
{
    memory_index Size;
    u8 *Base;
    memory_index Used;
};

inline void
initialize_arena(memory_arena *Arena, memory_index Size, void *Base)
{
    Arena->Size = Size;
    Arena->Base = (u8 *)Base;
    Arena->Used = 0;
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))
inline void *
PushSize_(memory_arena *Arena, memory_index Size)
{
    if(!((Arena->Used + Size) <= Arena->Size))
    {
        ThrowError("MemoryArena Overflow!");
    }
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    return(Result);
}

#include "logl_main.h"
#include "logl_shader.h"
#include "logl_camera.h"
#include "logl_opengl.h"
#include "logl_gltf.h"

enum state
{
    DEBUG,
    PLAY,
};

struct Game_State
{
    memory_arena world_arena;

    VABO opengl_buffer;

    state state;

    my_camera debug_camera;
};

#define LOGL_H
#endif

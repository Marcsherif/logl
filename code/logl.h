#if !defined(LOGL_H)

#include "logl_platform.h"

#define minimum(a, b) ((a < b) ? (a) : (b))
#define maximum(a, b) ((a > b) ? (a) : (b))

struct Memory_Arena
{
    Memory_Index size;
    u8 *base;
    Memory_Index used;
};

inline void
initialize_arena(Memory_Arena *arena, Memory_Index size, void *base)
{
    arena->size = size;
    arena->base = (u8 *)base;
    arena->used = 0;
}

#define push_struct(arena, type) (type *)push_size(arena, sizeof(type))
#define push_array(arena, count, type) (type *)push_size(arena, (count)*sizeof(type))
inline void *
push_size(Memory_Arena *arena, Memory_Index size)
{
    if(!((arena->used + size) <= arena->size))
    {
        throw_error("MemoryArena Overflow!");
    }
    void *result = arena->base + arena->used;
    arena->used += size;
    return(result);
}

#include "logl_camera.h"
#include "logl_opengl.h"
#include "logl_gltf.h"
#include "logl_livelink.h"

enum App_Mode
{
    workbench_mode,
    free_camera_mode,
};

struct Character_State
{
    Gltf_Model model = {};
    Live_Link_State live_link;

    bool default_pose = false;
    bool paused = false;
    bool live_link_enabled = false;
    bool reset_pose = false;
    bool seek_animation = false;
    i32 selected_clip = 0;
    f32 playback_rate = 1.0f;
    f32 animation_time = 0.0f;
    f32 animation_cursor = 0.0f;
};

struct Game_State
{
    Memory_Arena world_arena;

    Vertex_Buffers opengl_buffer;

    App_Mode mode;

    Camera debug_camera;

    Character_State character;
    Workbench_Settings dbg_gui;
    f32 light_time = 0;

    u32 light_shader = 0;
    u32 source_light_shader = 0;
    u32 terrain_shader = 0;
    u32 anim_shader = 0;
    u32 light_source_cube = 0;
    u32 terrain = 0;
    Texture terrain_texture = {};
};

#define LOGL_H
#endif

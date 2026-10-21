#if !defined(LOGL_OPENGL_H)

struct Texture
{
    u32 index;
    i32 height;
    i32 width;
};

struct Vertex_Buffers
{
    u32 *vao;
    // NOTE(marc): If using an ebo this is index count else its vertex count
    u32 *count;
    u32 n_vao;

    u32 *vbo;
    u32 n_vbo;

    u32 *ebo;
    u32 n_ebo;

    // TODO: turn these into arrays when needed
    u32 num_strips;
    u32 num_verts_per_strip;
    // TODO: get rid of this
    u32 rez;
};

#define LOGL_OPENGL_H
#endif

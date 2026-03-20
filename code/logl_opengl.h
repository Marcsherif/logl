#if !defined(LOGL_OPENGL_H)

struct Texture
{
    u32 index;
    i32 height;
    i32 width;
};

struct VABO
{
    u32 *VAO;
    // NOTE(marc): If using an EBO this is index count else its vertex count
    u32 *count;
    u32 nVAO;

    u32 *VBO;
    u32 nVBO;

    u32 *EBO;
    u32 nEBO;

    // TODO: turn these into arrays when needed
    u32 NUM_STRIPS;
    u32 NUM_VERTS_PER_STRIP;
    // TODO: get rid of this
    u32 rez;
};

#define LOGL_OPENGL_H
#endif

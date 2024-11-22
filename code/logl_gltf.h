#if !defined(LOGL_GLTF_H)

struct gltf_vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

struct gltf_texture
{
    u32 id;
    u32 index;
    char *type;
};

struct gltf_primitive
{
    gltf_vertex *vertices;
    u32 nVerts;

    u32 *indices;
    u32 nIndices;

    u32 *textureIndex;
    u32 nTextures;

    u32 VAO;
    u32 VBO;
    u32 EBO;
};

struct gltf_node
{
    gltf_primitive *primitive;

    glm::mat4 transform;
    gltf_node *parent;
    gltf_node *child;
};

struct gltf_model
{
    gltf_node *root;

    gltf_texture *textures;
    u32 nTextures;

    u32 nVerts;
    u32 nIndices;
};

#define LOGL_GLTF_H
#endif

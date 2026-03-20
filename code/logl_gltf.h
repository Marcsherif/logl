#if !defined(LOGL_GLTF_H)

#define MAX_BONE_INFLUENCE 4

struct gltf_vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::ivec4 jointIds; // bone indices that affect this vert
    glm::vec4 weights; // weights from each bone
};

#define TEXTURE_CAPACITY 100
struct gltf_texture
{
    u32 id;
    u32 index;
    void *tex_id;
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
    char name[64];
    gltf_node *parent;

    glm::vec3 translation;
    glm::quat rotation;
    glm::vec3 scale;

    // live view targets
    glm::vec3 targetTranslation;
    glm::quat targetRotation;

    b32 useBakedMatrix;
    glm::mat4 localTransform;
    glm::mat4 globalTransform;

    gltf_primitive *primitive;
    u32 nPrimitives;

    gltf_node **children;
    size_t childCount;
};

struct gltf_skin
{
    char name[64];
    u32 jointCount;
    u32 *jointNodeIndices;      // Indices into flat model->nodes array
    glm::mat4 *inverseBindMatrices;
};

enum interpolation_type
{
    linear,
    step,
    cubic_spline,
    interpolation_type_max_enum,
};
struct gltf_sampler
{
    u32 count;
    float* input;     // timestamps
    void* output;      // vec3 or Quat data
    interpolation_type interpolation;
};

enum animation_type
{
    invalid,
    translation,
    rotation,
    scale,
    animation_type_max_enum,
};

struct gltf_channel
{
    u32 nodeIndex;
    gltf_sampler *sampler; // Pointer to the keyframe data
    animation_type path;   // "translation", "rotation", or "scale"
};

struct gltf_animation
{
    char name[64];
    float duration;
    u32 samplerCount;
    gltf_sampler *samplers;
    u32 channelCount;
    gltf_channel *channels;
};

struct gltf_model
{
    u32 *rootIndices;
    u32 nRoots;

    gltf_node *nodes;
    size_t nodeCount;

    char path[256];

    gltf_texture *textures;
    u32 nTextures;

    u32 nVerts;
    u32 nIndices;

    glm::mat4 finalBonesMatrices[100];
    gltf_skin *skins;
    u32 nSkins;

    gltf_animation *animations;
    f32 animationTime;
    u32 nAnimations;
};

#define LOGL_GLTF_H
#endif

#if !defined(LOGL_GLTF_H)

#define MAX_BONE_INFLUENCE 4

struct Gltf_Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;
    glm::ivec4 joint_ids; // bone indices that affect this vert
    glm::vec4 weights; // weights from each bone
};

#define TEXTURE_CAPACITY 100
struct Gltf_Texture
{
    u32 id;
    u32 index;
    void *tex_id;
    char *type;
};

struct Gltf_Primitive
{
    Gltf_Vertex *vertices;
    u32 n_verts;

    u32 *indices;
    u32 n_indices;

    u32 *texture_index;
    u32 n_textures;

    u32 vao;
    u32 vbo;
    u32 ebo;
};

struct Gltf_Node
{
    char name[64];
    Gltf_Node *parent;

    glm::vec3 translation;
    glm::quat rotation;
    glm::vec3 scale;

    // rest pose
    glm::vec3 rest_translation;
    glm::quat rest_rotation;
    glm::vec3 rest_scale;
    glm::mat4 rest_local_transform;

    // live view targets
    glm::vec3 target_translation;
    glm::quat target_rotation;
    b32 has_live_target;

    b32 use_baked_matrix;
    glm::mat4 local_transform;
    glm::mat4 global_transform;

    Gltf_Primitive *primitive;
    u32 n_primitives;

    Gltf_Node **children;
    size_t child_count;
};

struct Gltf_Skin
{
    char name[64];
    u32 joint_count;
    u32 *joint_node_indices;      // Indices into flat model->nodes array
    glm::mat4 *inverse_bind_matrices;
};

enum Interpolation_Type
{
    linear,
    step,
    cubic_spline,
    interpolation_type_max_enum,
};
struct Gltf_Sampler
{
    u32 count;
    float* input;     // timestamps
    void* output;      // vec3 or Quat data
    Interpolation_Type interpolation;
};

enum Animation_Type
{
    invalid,
    translation,
    rotation,
    scale,
    animation_type_max_enum,
};

struct Gltf_Channel
{
    u32 node_index;
    Gltf_Sampler *sampler; // Pointer to the keyframe data
    Animation_Type path;   // "translation", "rotation", or "scale"
};

struct Gltf_Animation
{
    char name[64];
    float duration;
    u32 sampler_count;
    Gltf_Sampler *samplers;
    u32 channel_count;
    Gltf_Channel *channels;
};

#include <unordered_map>
#include <string>
struct Gltf_Model
{
    u32 *root_indices;
    u32 n_roots;

    Gltf_Node *nodes;
    size_t node_count;

    char path[256];

    Gltf_Texture *textures;
    u32 n_textures;

    u32 n_verts;
    u32 n_indices;

    glm::mat4 final_bones_matrices[100];
    Gltf_Skin *skins;
    u32 n_skins;

    Gltf_Animation *animations;
    u32 n_animations;

    std::unordered_map<std::string, u32> bone_mapping;
};

#define LOGL_GLTF_H
#endif

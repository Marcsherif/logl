#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;
layout(location = 3) in ivec4 bone_ids;
layout(location = 4) in vec4 weights;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const int max_bones = 100;
const int max_bone_influence = 4;
uniform mat4 final_bones_matrices[max_bones];

out vec3 frag_pos;
out vec3 normal;
out vec2 tex_coords;
out float joint_weight;
uniform int selected_joint;

void main()
{
    vec4 total_position = vec4(0.0f);
    vec3 total_normal = vec3(0.0f);
    joint_weight = 0.0;

    for(int i = 0 ; i < max_bone_influence ; i++)
    {
        if(bone_ids[i] == -1)
            continue;
        if(bone_ids[i] == selected_joint)
            joint_weight += weights[i];
        if(bone_ids[i] >=max_bones)
        {
            total_position = vec4(pos,1.0f);
            break;
        }
        vec4 local_position = final_bones_matrices[bone_ids[i]] * vec4(pos,1.0f);
        total_position += local_position * weights[i];

        vec3 local_normal = mat3(final_bones_matrices[bone_ids[i]]) * norm;
        total_normal += local_normal * weights[i];
    }

    normal = mat3(transpose(inverse(model))) * total_normal;
    frag_pos = vec3(model * total_position);

    mat4 view_model = view * model;
    gl_Position =  projection * view_model * total_position;
    tex_coords = tex;
}

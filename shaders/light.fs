#version 330 core
out vec4 frag_color;
in float joint_weight;
uniform bool show_weights;

struct Material {
    sampler2D diffuse1;
    sampler2D specular1;
    sampler2D emission;
    float shininess;
};
uniform Material material;

struct Dir_Light {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform Dir_Light dir_light;

struct Point_Light {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NR_POINT_LIGHTS 4
uniform Point_Light point_lights[NR_POINT_LIGHTS];

struct Spot_Light {
    vec3 position;
    vec3 direction;
    float cut_off;
    float outer_cut_off;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
uniform Spot_Light spot_light;

in vec3 frag_pos;
in vec3 normal;
in vec2 tex_coords;

uniform vec3 view_pos;

vec3 calc_dir_light(Dir_Light light, vec3 normal, vec3 view_dir)
{
    vec3 light_dir = normalize(-light.direction);

    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);

    // specular shading
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse1, tex_coords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse1, tex_coords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular1, tex_coords));

    return (ambient + diffuse + specular);
}


vec3 calc_point_light(Point_Light light, vec3 normal, vec3 frag_pos, vec3 view_dir)
{
    vec3 light_dir = normalize(light.position - frag_pos);

    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);

    // specular shading
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

    // attenuation
    float distance    = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
  			            light.quadratic * (distance * distance));

    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse1, tex_coords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse1, tex_coords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular1, tex_coords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 calc_spot_light(Spot_Light spot_light, vec3 norm, vec3 frag_pos, vec3 view_dir)
{
    // diffuse
    vec3 light_dir = normalize(spot_light.position - frag_pos);
    //vec3 lightDir = normalize(-spotLight.direction);
    float diff = max(dot(norm, light_dir), 0.0);

    // specular
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

    // emission
    vec3 spec_texel_color = vec3(texture(material.specular1, tex_coords));
    vec3 emission = (1.0f - sign(spec_texel_color)) * vec3(texture(material.emission, tex_coords));

    // attenuation
    float distance    = length(spot_light.position - frag_pos);
    float attenuation = 1.0 / (spot_light.constant + spot_light.linear * distance + spot_light.quadratic * sqrt(distance));

    // combine results
    vec3 ambient = spot_light.ambient * vec3(texture(material.diffuse1, tex_coords));
    vec3 diffuse = spot_light.diffuse * diff * vec3(texture(material.diffuse1, tex_coords));
    vec3 specular = spot_light.specular * spec * vec3(texture(material.specular1, tex_coords));

    // spotlight (soft edges)
    float theta = dot(light_dir, normalize(-spot_light.direction));
    float epsilon = spot_light.cut_off - spot_light.outer_cut_off;
    float intensity = clamp((theta - spot_light.outer_cut_off) / epsilon, 0.0, 1.0);
    diffuse  *= intensity;
    specular *= intensity;

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse);
}

void main()
{
    if(show_weights)
    {
        float weight = clamp(joint_weight, 0.0, 1.0);
        vec3 cold = vec3(0.05, 0.1, 0.35);
        vec3 middle = vec3(0.0, 0.9, 0.4);
        vec3 hot = vec3(1.0, 0.2, 0.05);
        vec3 color = mix(cold, middle, clamp(weight * 2.0, 0.0, 1.0));
        color = mix(color, hot, clamp(weight * 2.0 - 1.0, 0.0, 1.0));
        frag_color = vec4(color, 1.0);
        return;
    }

    vec3 result = vec3(0.0);

    // properties
    vec3 norm = normalize(normal);
    vec3 view_dir = normalize(view_pos - frag_pos);

    // phase 1: Directional lighting
    result = calc_dir_light(dir_light, norm, view_dir);

    // phase 2: Point lights
    for(int i = 0; i < 1; i++)
        result = calc_point_light(point_lights[i], norm, frag_pos, view_dir);

    // phase 3: Spot light
    result += calc_spot_light(spot_light, norm, frag_pos, view_dir);

    frag_color = vec4(result, 1.0);
}

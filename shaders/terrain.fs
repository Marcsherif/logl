#version 410 core

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

uniform float scale;
uniform float shift;

uniform float shader_r;
uniform float shader_g;
uniform float shader_b;

uniform vec3 view_pos;
uniform mat4 model;

uniform sampler2D height_map;
in float height;
in vec3 frag_pos;
in vec2 tex_coords;
in vec2 u_texel_size;
in vec3 norm;

out vec4 frag_color;

vec3 calc_dir_light(Dir_Light light, vec3 normal, vec3 view_dir, vec3 color)
{
    vec3 light_dir = normalize(-light.direction);

    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);

    // specular shading
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

    // combine results
    vec3 ambient  = light.ambient  * color; // vec3(texture(material.diffuse1, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * color; // vec3(texture(material.diffuse1, TexCoords));
    vec3 specular = light.specular * spec * color *2; //vec3(texture(material.specular1, TexCoords));

    return (ambient + diffuse + specular);
}


vec3 calc_point_light(Point_Light light, vec3 normal, vec3 frag_pos, vec3 view_dir, vec3 color)
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
    vec3 ambient  = light.ambient  * color; // vec3(texture(material.diffuse1, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * color; // vec3(texture(material.diffuse1, TexCoords));
    vec3 specular = light.specular * spec * color; // vec3(texture(material.specular1, TexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 calc_spot_light(Spot_Light spot_light, vec3 norm, vec3 frag_pos, vec3 view_dir, vec3 color)
{
    // diffuse
    vec3 light_dir = normalize(spot_light.position - frag_pos);
    //vec3 lightDir = normalize(-spotLight.direction);
    float diff = max(dot(norm, light_dir), 0.0);

    // specular
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

    // emission
    vec3 spec_texel_color = color*2; //vec3(texture(material.specular1, TexCoords));
    vec3 emission = (1.0f - sign(spec_texel_color)) *color; //  vec3(texture(material.emission, TexCoords));

    // attenuation
    float distance    = length(spot_light.position - frag_pos);
    float attenuation = 1.0 / (spot_light.constant + spot_light.linear * distance + spot_light.quadratic * sqrt(distance));

    // combine results
    vec3 ambient = spot_light.ambient * color; // vec3(texture(material.diffuse1, TexCoords));
    vec3 diffuse = spot_light.diffuse * diff * color; //vec3(texture(material.diffuse1, TexCoords));
    vec3 specular = spot_light.specular * spec * color*2; // vec3(texture(material.specular1, TexCoords));

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
    float h = (height + shift)/scale;
    vec4 color = vec4(shader_r*h, shader_g*h, shader_b*h, 1.0);
    //float left  = texture(heightMap, TexCoords + vec2(-uTexelSize.x, 0.0)).y * scale * 2.0 - 1.0;
    //float right = texture(heightMap, TexCoords + vec2( uTexelSize.x, 0.0)).y * scale * 2.0 - 1.0;
    //float up    = texture(heightMap, TexCoords + vec2(0.0,  uTexelSize.y)).y * scale * 2.0 - 1.0;
    //float down  = texture(heightMap, TexCoords + vec2(0.0, -uTexelSize.y)).y * scale * 2.0 - 1.0;
    //vec3 norm = normalize(vec3(down - up, 2.0, left - right));

    //vec3 result = vec3(0.0);

    // properties
    //vec3 viewDir = normalize(viewPos - FragPos);

    // phase 1: Directional lighting
    //result = CalcDirLight(dirLight, norm, viewDir, color.xyz);

    // phase 2: Point lights
    //for(int i = 0; i < 1; i++)
    //    result = CalcPointLight(pointLights[i], norm, FragPos, viewDir, color.xyz);

    // phase 3: Spot light
    //result += CalcSpotLight(spotLight, norm, FragPos, viewDir, color.xyz);

    frag_color = vec4(color.xyz, 1.0);
    //FragColor = vec4(shaderR*h, shaderG*h, shaderB*h, 1.0);
}

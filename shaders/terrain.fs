#version 410 core

struct Material {
    sampler2D diffuse1;
    sampler2D specular1;
    sampler2D emission;
    float shininess;
};
uniform Material material;

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
uniform SpotLight spotLight;

uniform float scale;
uniform float shift;

uniform float shaderR;
uniform float shaderG;
uniform float shaderB;

uniform vec3 viewPos;
uniform mat4 model;

uniform sampler2D heightMap;
in float Height;
in vec3 FragPos;
in vec2 TexCoords;
in vec2 uTexelSize;
in vec3 norm;

out vec4 FragColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 color)
{
    vec3 lightDir = normalize(-light.direction);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // combine results
    vec3 ambient  = light.ambient  * color; // vec3(texture(material.diffuse1, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * color; // vec3(texture(material.diffuse1, TexCoords));
    vec3 specular = light.specular * spec * color *2; //vec3(texture(material.specular1, TexCoords));

    return (ambient + diffuse + specular);
}


vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color)
{
    vec3 lightDir = normalize(light.position - fragPos);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // attenuation
    float distance    = length(light.position - fragPos);
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

vec3 CalcSpotLight(SpotLight spotLight, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 color)
{
    // diffuse
    vec3 lightDir = normalize(spotLight.position - FragPos);
    //vec3 lightDir = normalize(-spotLight.direction);
    float diff = max(dot(norm, lightDir), 0.0);

    // specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // emission
    vec3 specTexelColor = color*2; //vec3(texture(material.specular1, TexCoords));
    vec3 emission = (1.0f - sign(specTexelColor)) *color; //  vec3(texture(material.emission, TexCoords));

    // attenuation
    float distance    = length(spotLight.position - FragPos);
    float attenuation = 1.0 / (spotLight.constant + spotLight.linear * distance + spotLight.quadratic * sqrt(distance));

    // combine results
    vec3 ambient = spotLight.ambient * color; // vec3(texture(material.diffuse1, TexCoords));
    vec3 diffuse = spotLight.diffuse * diff * color; //vec3(texture(material.diffuse1, TexCoords));
    vec3 specular = spotLight.specular * spec * color*2; // vec3(texture(material.specular1, TexCoords));

    // spotlight (soft edges)
    float theta = dot(lightDir, normalize(-spotLight.direction));
    float epsilon = spotLight.cutOff - spotLight.outerCutOff;
    float intensity = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0);
    diffuse  *= intensity;
    specular *= intensity;

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse);
}

void main()
{
    float h = (Height + shift)/scale;
    vec4 color = vec4(shaderR*h, shaderG*h, shaderB*h, 1.0);
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

    FragColor = vec4(color.xyz, 1.0);
    //FragColor = vec4(shaderR*h, shaderG*h, shaderB*h, 1.0);
}

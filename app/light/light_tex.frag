#version 330 core

struct Material {
    sampler2D diffuse1;
    sampler2D specular1;
    vec3 emission_color;
    sampler2D emission1;
    float shininess;
};

uniform Material material;

struct Light {
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float cutOff;
    float outerCutOff;
};
uniform Light light;

out vec4 FragColor;

in vec4 gl_FragCoord;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform float time;

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirLight dirLight;

vec3 GetDiffuseColor(vec2 TexCoords) {
    return vec3(texture(material.diffuse1, TexCoords));
}

vec3 GetSpecularColor(vec2 TexCoords) {
    return vec3(texture(material.specular1, TexCoords));
}

vec3 GetEmissionColor(vec2 TexCoords) {
    return vec3(texture(material.emission1, TexCoords));
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);


    vec3 ambient = light.ambient * GetDiffuseColor(TexCoords);
    vec3 diffuse = light.diffuse * diff * GetDiffuseColor(TexCoords);
    vec3 specular = light.specular * spec * GetSpecularColor(TexCoords);

    return diffuse + specular;
    // return ambient + diffuse + specular;
};


struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 fragPos) {
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));


    vec3 ambient = light.ambient * GetDiffuseColor(TexCoords);
    vec3 diffuse = light.diffuse * diff * GetDiffuseColor(TexCoords);
    vec3 specular = light.specular * spec * GetSpecularColor(TexCoords);

    // return (ambient + diffuse + specular) * attenuation;
    return (diffuse + specular) * attenuation;
}

struct SpotLight {
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float cutOff;
    float outerCutOff;
};

uniform SpotLight spotLight;

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(viewDir);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambient = light.ambient * GetDiffuseColor(TexCoords);
    vec3 diffuse = light.diffuse * diff * GetDiffuseColor(TexCoords);
    vec3 specular = light.specular * spec * GetSpecularColor(TexCoords);

    return (diffuse + specular) * intensity;

}






void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(-FragPos);

    vec3 result = vec3(0.0f);

    // vec3 ambient = light.ambient * GetDiffuseColor(TexCoords);
    // result += ambient;

    result += CalcDirLight(dirLight, norm, viewDir);

    for(int i = 0; i < NR_POINT_LIGHTS; i++)
    result += CalcPointLight(pointLights[i], norm, viewDir, FragPos);
    // result += CalcPointLight(pointLights[1], norm, viewDir, FragPos);

    result += CalcSpotLight(spotLight, norm, viewDir);

    // invert specular light
    vec3 specularMap = (vec3(1.0-GetSpecularColor(TexCoords)) * 4.0f - 3.0f);
    specularMap = clamp(specularMap, 0.0, 1.0);
    float specularGray = (specularMap.r + specularMap.g + specularMap.b) / 3.0f;
    // vec3 specular = light.specular * specular_magnitude * (specularGray * GetDiffuseColor(TexCoords)) ;
    vec3 emission = specularGray * vec3(GetEmissionColor(TexCoords));

    result = max(result, emission);

    FragColor = vec4(result, 1.0);
}

#version 330 core

struct PBRMaterial {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;

    sampler2D texAlbedo;
    sampler2D texMetallic;
    sampler2D texRoughness;
    sampler2D texAO;

    bool useTexAlbedo;
    bool useTexMetallic;
    bool useTexRoughness;
    bool useTexAO;
};

struct PBRMatParams {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
};

struct PBRDirLight {
    bool enabled;

    vec3 direction;
    vec3 color;
};

struct PBRPointLight {
    bool enabled;

    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 color;
};

struct PBRSpotLight {
    bool enabled;

    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 color;
};

out vec4 fragColor;

in vec3 worldPos;
in vec3 normal;
in vec2 texCoord;

uniform vec3 viewPos;

uniform PBRMaterial mat;

const float PI = 3.14159265359;

vec3 fresnelSchlinck(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pos(1.0 - cosTheta, 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}


vec3 calcRadiance(vec3 N, vec3 V, vec3 L, vec3 F0, vec3 radiance, PBRMatParams params) {

    vec3 H = normalize(V + L);

    float NDF = distributionGGX(N, H, params.roughness);
    float G = geometrySmith(N, V, L, params.roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular = numerator / max(denominator, 0.001);

    kD *= 1.0 - params.metallic;

    float NdotL = max(dot(N, L), 0.0);
    return (kD * params.albedo / PI + specular) * radiance * NdotL;
}

vec3 calcDirLight(PBRDirLight light, vec3 N, vec3 V, vec3 F0, PBRMatParams params) {
    vec3 L = normalize(-light.direction);
    vec3 radiance = light.color;

    return calcRadiance(N, V, L, F0, radiance, params);
}

vec3 calcPointLight(PBRPointLight light, vec3 N, vec3 V, vec3 F0, PBRMatParams params) {
    vec3 L = normalize(light.position - worldPos);
    float distance = length(light.position - worldPos);
    float attenuation = 1.0 / (distance * distance);
    // float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    vec3 radiance = light.color * attenuation;

    return calcRadiance(N, V, L, F0, radiance, params);
}

vec3 calcSpotLight(PBRSpotLight light, vec3 N, vec3 V, vec3 F0, PBRMatParams params) {
    vec3 L = normalize(light.position - worldPos);
    float distance = length(light.position - worldPos);
    float attenuation = 1.0 / (distance * distance);
    // float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    vec3 radiance = light.color * attenuation;

    float theta = dot(L, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    return intensity * calcRadiance(N, V, L, F0, radiance, params);
}

void main() {
    vec3 N = normalize(normal);
    vec3 V = normalize(viewPos - worldPos);

    PBRMatParams params;
    params.albedo = mat.useTexAlbedo? texture(mat.texAlbedo, texCoord) : mat.albedo;
    params.metallic = mat.useTexMetallic? texture(mat.texMetallic, texCoord) : mat.metallic;
    params.roughness = mat.useTexRoughness? texture(mat.texRoughness, texCoord) : mat.roughness;
    params.ao = mat.useTexAO? texture(mat.texAO, texCoord) : mat.texAO;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, params.albedo, params.metallic);

    vec3 Lo = vec3(0.0);
    if (dirLight.enabled) {
        Lo = calcDirLight(dirLight, N, V, F0, params);
    }

    for (int i = 0; i < numPointLights; ++i) {
        if (pointLights[i].enabled) {
            Lo += calcPointLight(pointLights[i], N, V, F0, params);
        }
    }

    for (int i = 0; i < numSpotLights; ++i) {
        if (spotLights[i].enabled) {
            Lo += calcSpotLight(spotLights[i], N, V, F0, params);
        }
    }

    vec3 ambient = vec3(0.03) * params.albedo * params.ao;
    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    fragColor = vec4(color, 1.0);
}


#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 lightSpaceMatrix;

out vec3 fragPos;
out vec3 normal;
out vec2 texCoord;

void main()
{
    vs_out.FragPos = vec3(model * vec4(inPos, 1.0));
    vs_out.Normal = mat3(transpose(inverse(model))) * inNormal;
    vs_out.TexCoords = inTexCoord;
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    gl_Position = proj * view * model * vec4(inPos, 1.0);
}

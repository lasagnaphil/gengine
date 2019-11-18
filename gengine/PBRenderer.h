//
// Created by lasagnaphil on 19. 9. 28..
//

#ifndef GENGINE_PBRENDERER_H
#define GENGINE_PBRENDERER_H

#define NUM_PBR_POINT_LIGHTS 16
#define NUM_PBR_SPOT_LIGHTS 8

#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"

#include <array>

// TODO: Add normal texture (needed for normal mapping)
struct PBRMaterial {
    /*
    glm::vec3 albedo;
    float __padding1;
    float metallic;
    float roughness;
    float ao;
    float __padding2;
     */

    Ref<Texture> texAlbedo;
    Ref<Texture> texMetallic;
    Ref<Texture> texRoughness;
    Ref<Texture> texAO;

    static Ref<PBRMaterial> quick(
            const std::string& albedo,
            const std::string& metallic,
            const std::string& roughness,
            const std::string& ao);
};

struct PBRDirLight {
    glm::vec3 direction;
    float __padding1;
    glm::vec3 color;
    float __padding2;

    uint32_t enabled = false;
    float __padding3;
    float __padding4;
    float __padding5;
};

struct PBRPointLight {
    glm::vec3 position;
    float __padding1;

    float constant;
    float linear;
    float quadratic;
    float __padding2;

    glm::vec3 color;
    float __padding3;

    uint32_t enabled = false;
    float __padding4;
    float __padding5;
    float __padding6;
};

struct PBRSpotLight {
    glm::vec3 position;
    float __padding1;
    glm::vec3 direction;
    float __padding2;

    float constant;
    float linear;
    float quadratic;
    float __padding3;

    glm::vec3 color;
    float __padding4;

    float cutOff;
    float outerCutOff;

    uint32_t enabled = false;
    float __padding5;
};

struct PBRCommand {
    Ref<Mesh> mesh;
    Ref<PBRMaterial> material;
    glm::mat4 modelMatrix;
};

class PBRenderer {
public:
    PBRenderer(Camera* camera = nullptr);

    void setCamera(Camera* camera) {
        this->camera = camera;
    }

    void init();

    void queueRender(const PBRCommand& command) {
        renderCommands.push_back(command);
    }

    void render();

    void renderImGui();

    PBRDirLight dirLight;
    std::array<PBRPointLight, NUM_PBR_POINT_LIGHTS> pointLights;
    std::array<PBRSpotLight, NUM_PBR_SPOT_LIGHTS> spotLights;

private:
    void renderPass(Ref<Shader> shader);

    // GLuint depthMapFBO;
    // GLuint depthMap;

    std::vector<PBRCommand> renderCommands;

    GLuint dirLightUBO;
    GLuint pointLightUBO;
    GLuint spotLightUBO;

    Camera* camera;

    // Rect3f dirLightProjVolume;
    // glm::ivec2 shadowFramebufferSize;

    // Ref<Shader> depthShader;
    Ref<Shader> pbrShader;
};

#endif //GENGINE_PBRENDERER_H

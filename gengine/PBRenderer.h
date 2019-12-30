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
#include "Colors.h"

#include <array>
#include <glmx/rect.h>

// TODO: Add normal texture (needed for normal mapping)
struct PBRMaterial {
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
    glm::vec3 color;
    uint32_t enabled = false;
};

struct PBRPointLight {
    glm::vec3 position;
    glm::vec3 color;
    uint32_t enabled = false;
};

struct PBRSpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 color;

    float cutOff;
    float outerCutOff;

    uint32_t enabled = false;
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

    void setShadowSettings(glmx::box projVolume, glm::ivec2 shadowFBSize) {
        this->dirLightProjVolume = projVolume;
        this->shadowFramebufferSize = shadowFBSize;
    }

    void init();

    void queueRender(const PBRCommand& command) {
        renderCommands.push_back(command);
    }

    void render();

    void renderImGui();

    PBRDirLight dirLight = {
            glm::normalize(glm::vec3 {2.0f, -3.0f, 2.0f}),
            {1.0f, 1.0f, 1.0f},
            true
    };

    std::array<PBRPointLight, NUM_PBR_POINT_LIGHTS> pointLights;
    std::array<PBRSpotLight, NUM_PBR_SPOT_LIGHTS> spotLights;

    glmx::box dirLightProjVolume = {
            {-10.f, -10.f, 0.f}, {10.f, 10.f, 1000.f}
    };
    glm::ivec2 shadowFramebufferSize = {2048, 2048};

private:
    void renderPass(Ref<Shader> shader);

    GLuint depthMapFBO;
    GLuint depthMap;

    std::vector<PBRCommand> renderCommands;

    GLuint dirLightUBO;
    GLuint pointLightUBO;
    GLuint spotLightUBO;

    Camera* camera;

    Ref<Shader> pbrShader;
    Ref<Shader> depthShader;
};

#endif //GENGINE_PBRENDERER_H

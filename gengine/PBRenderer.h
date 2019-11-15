//
// Created by lasagnaphil on 19. 9. 28..
//

#ifndef GENGINE_PBRENDERER_H
#define GENGINE_PBRENDERER_H

#define NUM_PBR_POINT_LIGHTS 16
#define NUM_PBR_SPOT_LIGHTS 8

#include "Shader.h"
#include "Texture.h"

// TODO: Add normal texture (needed for normal mapping)
struct PBRMaterial {
    glm::vec3 albedo;
    float metallic;
    float roughness;
    float ao;

    Ref<Texture> texAlbedo;
    Ref<Texture> texMetallic;
    Ref<Texture> texRoughness;
    Ref<Texture> texAO;
};

struct PBRDirLight {
    bool enabled;

    glm::vec3 direction;
    glm::vec3 color;
};

struct PBRPointLight {
    bool enabled;

    glm::vec3 position;

    float constant;
    float linear;
    float quadratic;

    glm::vec3 color;
};

struct PBRSpotLight {
    bool enabled;

    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    glm::vec3 color;
};

struct PBRCommand {
    Ref<Mesh> mesh;
    Ref<PBRMaterial> material;
    glm::mat4 modelMatrix;
};

class PBRenderer {
public:
    PBRenderer(Camera* camera = nullptr) :
        camera(camera) {

        dirLight.enabled = true;
        dirLight.direction = glm::normalize(glm::vec3 {2.0f, -3.0f, 2.0f});
        dirLight.color = glm::vec3(1.0f);
    }

    void init() {
        pbrShader = Shaders::pbr;
    }

    void render() {
        const unsigned int SCREEN_WIDTH = ImGui::GetIO().DisplaySize.x;
        const unsigned int SCREEN_HEIGHT = ImGui::GetIO().DisplaySize.y;
        float near_plane = 0.1f;
        float far_plane = 1000.0f;
    }

    void renderImGui() {
        ImGui::SetNextWindowPos(ImVec2(1620, 30), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(250, 250), ImGuiCond_FirstUseEver);

        ImGui::Begin("PhongRenderer Settings");

        if (ImGui::TreeNode("Directional Light Settings")) {
            ImGui::Checkbox("Enabled", &dirLight.enabled);

            ImGui::SliderFloat3("Direction", (float *) &dirLight.direction, -5.0f, 5.0f);
            ImGui::SliderFloat3("Color", (float *) &dirLight.color, 0.0f, 1.0f);

            /*
            ImGui::InputFloat2("Left/Right", (float *) &dirLightProjVolume.left);
            ImGui::InputFloat2("Bottom/Top", (float *) &dirLightProjVolume.bottom);
            ImGui::InputFloat2("zNear/zFar", (float *) &dirLightProjVolume.zNear);
            */

            ImGui::TreePop();
        }

        ImGui::End();
    }

private:
    void renderPass(Ref<Shader> shader) {
        shader->use();
        for (const auto& command : renderCommands) {
            shader->setMat4("model", command.modelMatrix);
            shader->setPBRMaterial(*command.material);

            glBindVertexArray(command.mesh->vao);
            if (command.mesh->indices.empty()) {
                glDrawArrays(GL_TRIANGLES, 0, command.mesh->vertices.size());
            }
            else {
                glDrawElements(GL_TRIANGLES, 0, command.mesh->indices.size(), GL_UNSIGNED_INT, 0);
            }
            glBindVertexArray(0);
        }
    }

    // GLuint depthMapFBO;
    // GLuint depthMap;

    std::vector<PBRCommand> renderCommands;

    PBRDirLight dirLight;
    std::array<PBRPointLight, NUM_PBR_POINT_LIGHTS> pointLights;
    std::array<PBRSpotLight, NUM_PBR_SPOT_LIGHTS> spotLights;

    Camera* camera;

    // Rect3f dirLightProjVolume;
    // glm::ivec2 shadowFramebufferSize;

    // Ref<Shader> depthShader;
    Ref<Shader> pbrShader;
};

#endif //GENGINE_PBRENDERER_H

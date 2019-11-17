//
// Created by lasagnaphil on 19. 11. 17..
//

#include "PBRenderer.h"

#include <imgui.h>

Ref<PBRMaterial>
PBRMaterial::quick(const std::string &albedo, const std::string &metallic, const std::string &roughness,
                   const std::string &ao) {

    Ref<Image> albedoImage = Image::fromFile(albedo);
    Ref<Image> metallicImage = Image::fromFile(metallic);
    Ref<Image> roughnessImage = Image::fromFile(roughness);
    Ref<Image> aoImage = Image::fromFile(ao);

    Ref<PBRMaterial> mat = Resources::make<PBRMaterial>();

    mat->texAlbedo = Texture::fromImage(albedoImage);
    mat->texMetallic = Texture::fromImage(metallicImage);
    mat->texRoughness = Texture::fromImage(roughnessImage);
    mat->texAO = Texture::fromImage(aoImage);

    albedoImage->dispose();
    metallicImage->dispose();
    roughnessImage->dispose();
    aoImage->dispose();

    return mat;
}
PBRenderer::PBRenderer(Camera *camera) :
        camera(camera) {

    dirLight.enabled = true;
    dirLight.direction = glm::normalize(glm::vec3 {2.0f, -3.0f, 2.0f});
    dirLight.color = glm::vec3(1.0f);
}

void PBRenderer::init() {
    pbrShader = Shaders::pbr;
}

void PBRenderer::render() {
    const unsigned int SCREEN_WIDTH = ImGui::GetIO().DisplaySize.x;
    const unsigned int SCREEN_HEIGHT = ImGui::GetIO().DisplaySize.y;
    float near_plane = 0.1f;
    float far_plane = 1000.0f;

    pbrShader->use();
    if (camera) {
        pbrShader->setCamera(camera);
    }

    pbrShader->setBool("dirLight.enabled", dirLight.enabled);
    if (dirLight.enabled) {
        pbrShader->setVec3("dirLight.direction", dirLight.direction);
        pbrShader->setVec3("dirLight.color", dirLight.color);
    }
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


    for (int i = 0; i < NUM_PBR_POINT_LIGHTS; i++) {
        pbrShader->setBool("pointLights[i].enabled", pointLights[i].enabled);
        if (pointLights[i].enabled) {
            pbrShader->setVec3("pointLights[i].position", pointLights[i].position);
            pbrShader->setFloat("pointLights[i].constant", pointLights[i].constant);
            pbrShader->setFloat("pointLights[i].linear", pointLights[i].linear);
            pbrShader->setFloat("pointLights[i].quadratic", pointLights[i].quadratic);
            pbrShader->setVec3("pointLights[i].color", pointLights[i].color);
        }
    }

    for (int i = 0; i < NUM_PBR_SPOT_LIGHTS; i++) {
        pbrShader->setBool("spotLights[i].enabled", spotLights[i].enabled);
        if (spotLights[i].enabled) {
            pbrShader->setVec3("spotLights[i].position", spotLights[i].position);
            pbrShader->setVec3("spotLights[i].direction", spotLights[i].direction)
        }
    }

    renderPass(pbrShader);
}

void PBRenderer::renderImGui() {
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

void PBRenderer::renderPass(Ref<Shader> shader) {
    shader->use();
    for (const auto& command : renderCommands) {
        shader->setMat4("model", command.modelMatrix);
        shader->setPBRMaterial(*command.material);

        glBindVertexArray(command.mesh->vao);
        if (command.mesh->indices.empty()) {
            glDrawArrays(GL_TRIANGLES, 0, command.mesh->vertices.size());
        }
        else {
            glDrawElements(GL_TRIANGLES, command.mesh->indices.size(), GL_UNSIGNED_INT, 0);
        }
        glBindVertexArray(0);
    }
}



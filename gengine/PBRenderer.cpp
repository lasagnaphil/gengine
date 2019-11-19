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

}

void PBRenderer::init() {
    pbrShader = Shaders::pbr;

    pbrShader->use();
    /*
    glUniformBlockBinding(pbrShader->program, 0, 0);
    glUniformBlockBinding(pbrShader->program, 1, 1);
    glUniformBlockBinding(pbrShader->program, 2, 2);

    glGenBuffers(1, &dirLightUBO);
    glGenBuffers(1, &pointLightUBO);
    glGenBuffers(1, &spotLightUBO);

    glBindBuffer(GL_UNIFORM_BUFFER, dirLightUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(PBRDirLight), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_UNIFORM_BUFFER, pointLightUBO);
    glBufferData(GL_UNIFORM_BUFFER, NUM_PBR_POINT_LIGHTS * sizeof(PBRPointLight), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_UNIFORM_BUFFER, spotLightUBO);
    glBufferData(GL_UNIFORM_BUFFER, NUM_PBR_SPOT_LIGHTS * sizeof(PBRSpotLight), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, dirLightUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, pointLightUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, spotLightUBO);
     */
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
    else {
        std::cerr << "Error in PBRenderer: camera not set" << std::endl;
    }

    /*
    glBindBuffer(GL_UNIFORM_BUFFER, dirLightUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PBRDirLight), &dirLight);

    glBindBuffer(GL_UNIFORM_BUFFER, pointLightUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, NUM_PBR_POINT_LIGHTS * sizeof(PBRPointLight), &pointLights);

    glBindBuffer(GL_UNIFORM_BUFFER, spotLightUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, NUM_PBR_SPOT_LIGHTS * sizeof(PBRSpotLight), &spotLights);
     */

    pbrShader->setBool("dirLight.enabled", dirLight.enabled);
    if (dirLight.enabled) {
        pbrShader->setVec3("dirLight.direction", dirLight.direction);
        pbrShader->setVec3("dirLight.color", dirLight.color);
    }

    for (int i = 0; i < NUM_PBR_POINT_LIGHTS; i++) {
        std::string lname = std::string("pointLights[") + std::to_string(i) + "]";
        pbrShader->setBool((lname + ".enabled").c_str(), pointLights[i].enabled);
        if (pointLights[i].enabled) {
            pbrShader->setVec3((lname + ".position").c_str(), pointLights[i].position);
            pbrShader->setVec3((lname + ".color").c_str(), pointLights[i].color);
        }
    }

    for (int i = 0; i < NUM_PBR_SPOT_LIGHTS; i++) {
        std::string lname = std::string("spotLights[") + std::to_string(i) + "]";
        pbrShader->setBool((lname + ".enabled").c_str(), spotLights[i].enabled);
        if (spotLights[i].enabled) {
            pbrShader->setVec3((lname + ".position").c_str(), spotLights[i].position);
            pbrShader->setVec3((lname + ".direction").c_str(), spotLights[i].direction);
            pbrShader->setVec3((lname + ".color").c_str(), spotLights[i].color);
        }
    }

    renderPass(pbrShader);

    renderCommands.clear();
}

void PBRenderer::renderImGui() {
    ImGui::SetNextWindowPos(ImVec2(1620, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(250, 250), ImGuiCond_FirstUseEver);

    ImGui::Begin("PhongRenderer Settings");

    if (ImGui::TreeNode("Directional Light Settings")) {
        ImGui::Checkbox("Enabled", (bool*) &dirLight.enabled);

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



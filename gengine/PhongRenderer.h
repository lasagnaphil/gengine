//
// Created by lasagnaphil on 19. 3. 17.
//

#ifndef MOTION_EDITING_PHONGRENDERER_H
#define MOTION_EDITING_PHONGRENDERER_H

#include "Mesh.h"
#include "Material.h"

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>

struct Rect3f {
    float left, right, bottom, top, zNear, zFar;
};

struct RenderCommand {
    Ref<Mesh> mesh;
    Ref<Material> material;
    glm::mat4 modelMatrix;
};

struct DirLight {
    bool enabled = false;

    glm::vec3 direction = {};

    glm::vec4 ambient = {};
    glm::vec4 diffuse = {};
    glm::vec4 specular = {};

    float intensity = 32.0f;
};

class PhongRenderer {
public:
    bool viewDepthBufferDebug = false;

    PhongRenderer(Rect3f projVolume = {-5.f, 5.f, -5.f, 5.f, 0.f, 1000.f},
            glm::ivec2 shadowFBSize = {1024, 1024},
            Camera* camera = nullptr) :

            camera(camera), shadowFramebufferSize(shadowFBSize),
            dirLightProjVolume(projVolume) {

        dirLight.enabled = true;
        dirLight.direction = glm::normalize(glm::vec3 {2.0f, -3.0f, 2.0f});
        dirLight.ambient = {0.3f, 0.3f, 0.3f, 0.3f};
        dirLight.diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
        dirLight.specular = {1.0f, 1.0f, 1.0f, 1.0f};
        dirLight.intensity = 0.5f;
    }

    void setCamera(Camera* camera) {
        this->camera = camera;
    }

    void setDefaultFBO(GLuint FBO) {
        defaultFBO = FBO;
    }

    void init() {
        depthShader = Shaders::depth;
        debugDepthShader = Shaders::depthDebug;
        phongShader = Shaders::phong;

        glGenFramebuffers(1, &depthMapFBO);

        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                shadowFramebufferSize.x, shadowFramebufferSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);

        phongShader->use();
        phongShader->setInt("diffuseTexture", 0);
        phongShader->setInt("shadowMap", 1);
        debugDepthShader->use();
        debugDepthShader->setInt("depthMap", 0);
    }

    void queueRender(const RenderCommand& command) {
        renderCommands.push_back(command);
    }

    void render() {
        // renderPass();

        const unsigned int SCREEN_WIDTH = ImGui::GetIO().DisplaySize.x;
        const unsigned int SCREEN_HEIGHT = ImGui::GetIO().DisplaySize.y;
        float near_plane = 0.1f;
        float far_plane = 1000.f;

        glm::mat4 dirLightProjection = glm::ortho(dirLightProjVolume.left, dirLightProjVolume.right,
                dirLightProjVolume.bottom, dirLightProjVolume.top, dirLightProjVolume.zNear, dirLightProjVolume.zFar);

        glm::vec3 dirLightPos = -glm::normalize(dirLight.direction) * dirLightProjVolume.zFar * 0.5f;
        glm::mat4 dirLightView = glm::lookAt(dirLightPos, glm::vec3(0.0f), {0.0f, 1.0f, 0.0f});
        glm::mat4 dirLightSpaceMatrix = dirLightProjection * dirLightView;

        depthShader->use();
        depthShader->setMat4("dirLightSpaceMatrix", dirLightSpaceMatrix);

        glViewport(0, 0, shadowFramebufferSize.x, shadowFramebufferSize.y);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        {
            glClear(GL_DEPTH_BUFFER_BIT);
            glCullFace(GL_FRONT);
            renderPass(depthShader);
            glCullFace(GL_BACK);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);

        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        phongShader->use();

        if (camera) {
            phongShader->setCamera(camera);
        }
        else {
            std::cerr << "Error in PhongRenderer: camera not set" << std::endl;
        }

        phongShader->setBool("dirLight.enabled", dirLight.enabled);
        phongShader->setVec3("dirLight.direction", dirLight.direction);
        phongShader->setVec4("dirLight.ambient", dirLight.ambient);
        phongShader->setVec4("dirLight.diffuse", dirLight.diffuse);
        phongShader->setVec4("dirLight.specular", dirLight.specular);
        phongShader->setFloat("dirLight.intensity", dirLight.intensity);
        phongShader->setMat4("dirLightSpaceMatrix", dirLightSpaceMatrix);

        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        phongShader->setInt("shadowMap", 8);

        renderPass(phongShader);

        if (viewDepthBufferDebug) {
            debugDepthShader->use();
            debugDepthShader->setFloat("near_plane", near_plane);
            debugDepthShader->setFloat("far_plane", far_plane);
            glActiveTexture(GL_TEXTURE8);
            glBindTexture(GL_TEXTURE_2D, depthMap);
            debugDepthShader->setInt("depthMap", 8);
            renderDepthMapDebug();
        }

        renderCommands.clear();
    }

    void renderImGui() {

        ImGui::SetNextWindowPos(ImVec2(1620, 30), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(250, 250), ImGuiCond_FirstUseEver);

        ImGui::Begin("PhongRenderer Settings");

        if (ImGui::TreeNode("Directional Light Settings")) {
            ImGui::Checkbox("Enabled", &dirLight.enabled);

            ImGui::SliderFloat3("Direction", (float *) &dirLight.direction, -5.0f, 5.0f);
            ImGui::SliderFloat3("Ambient", (float *) &dirLight.ambient, 0.0f, 1.0f);
            ImGui::SliderFloat3("Diffuse", (float *) &dirLight.diffuse, 0.0f, 1.0f);
            ImGui::SliderFloat3("Specular", (float *) &dirLight.specular, 0.0f, 1.0f);
            ImGui::SliderFloat("Intensity", (float *) &dirLight.intensity, 0.0f, 1.0f);

            ImGui::InputFloat2("Left/Right", (float *) &dirLightProjVolume.left);
            ImGui::InputFloat2("Bottom/Top", (float *) &dirLightProjVolume.bottom);
            ImGui::InputFloat2("zNear/zFar", (float *) &dirLightProjVolume.zNear);

            ImGui::TreePop();
        }

        ImGui::End();
    }

private:
    void renderPass(Ref<Shader> shader) {
        shader->use();
        for (const RenderCommand& command : renderCommands) {
            shader->setMat4("model", command.modelMatrix);
            shader->setMaterial(*command.material);

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

    void renderDepthMapDebug() {
        static GLuint quadVAO = 0;
        static GLuint quadVBO;
        if (quadVAO == 0) {
            float quadVertices[] = {
                    // positions        // texture Coords
                    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                    1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            };
            // setup plane VAO
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        }
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

    GLuint defaultFBO = 0;

    GLuint depthMapFBO;
    GLuint depthMap;

    std::vector<RenderCommand> renderCommands;

    DirLight dirLight;
    Camera* camera;

    Rect3f dirLightProjVolume;
    glm::ivec2 shadowFramebufferSize;

    Ref<Shader> depthShader;
    Ref<Shader> phongShader;
    Ref<Shader> debugDepthShader;
};

#endif //MOTION_EDITING_PHONGRENDERER_H

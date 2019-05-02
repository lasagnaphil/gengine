//
// Created by lasagnaphil on 19. 3. 17.
//

#ifndef MOTION_EDITING_PHONGRENDERER_H
#define MOTION_EDITING_PHONGRENDERER_H

#include "Mesh.h"
#include "Material.h"

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
    const static unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

public:

    bool viewDepthBufferDebug = false;

    PhongRenderer() {
        dirLight.enabled = true;
        dirLight.direction = {-1.0f, -1.0f, 1.0f};
        dirLight.ambient = {0.5f, 0.5f, 0.5f, 1.0f};
        dirLight.diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
        dirLight.specular = {1.0f, 1.0f, 1.0f, 1.0f};
        dirLight.intensity = 1.0f;
    }

    void init() {
        depthShader = Resources::make<Shader>("gengine/shaders/depth.vert", "gengine/shaders/depth.frag");
        depthShader->compile();

        phongShader = Shaders::phong;

        debugDepthShader = Resources::make<Shader>(
                "gengine/shaders/depth_debug.vert", "gengine/shaders/depth_debug.frag");
        debugDepthShader->compile();

        glGenFramebuffers(1, &depthMapFBO);

        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        debugDepthShader->use();
        debugDepthShader->setInt("depthMap", 0);
    }

    void queueRender(const RenderCommand& command) {
        renderCommands.push_back(command);
    }

    void render() {
        for (Ref<Shader> ref : {phongShader}) {
            auto* shader = ref.get();
			shader->use();
            shader->setBool("dirLight.enabled", dirLight.enabled);
            shader->setVec3("dirLight.direction", dirLight.direction);
            shader->setVec4("dirLight.ambient", dirLight.ambient);
            shader->setVec4("dirLight.diffuse", dirLight.diffuse);
            shader->setVec4("dirLight.specular", dirLight.specular);
            shader->setFloat("dirLight.intensity", dirLight.intensity);
        }

        // renderPass();

        float near_plane = 0.1f;
        float far_plane = 200.f;

        glm::mat4 dirLightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::vec3 dirLightPos = dirLight.direction * -100.0f;
        glm::mat4 dirLightView = glm::lookAt(dirLightPos, glm::vec3(0.0f), {0.0f, 1.0f, 0.0f});
        glm::mat4 dirLightSpaceMatrix = dirLightProjection * dirLightView;

        depthShader->use();
        depthShader->setMat4("lightSpaceMatrix", dirLightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        {
            glClear(GL_DEPTH_BUFFER_BIT);
            renderPass(depthShader);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, (int) ImGui::GetIO().DisplaySize.x, (int) ImGui::GetIO().DisplaySize.y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (viewDepthBufferDebug) {
            debugDepthShader->use();
            debugDepthShader->setFloat("near_plane", near_plane);
            debugDepthShader->setFloat("far_plane", far_plane);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, depthMap);
            renderDepthMapDebug();
        }
        else {
            renderPass(phongShader);
        }

        renderCommands.clear();
    }

private:
    void renderPass(Ref<Shader> shader) {
        shader->use();
        for (const RenderCommand& command : renderCommands) {
            shader->setMat4("model", command.modelMatrix);
            shader->setMaterial(*command.material);
            if (command.material->texDiffuse) {
                glActiveTexture(GL_TEXTURE0);
                command.material->texDiffuse->bind();
            }
            if (command.material->texSpecular) {
                glActiveTexture(GL_TEXTURE1);
                command.material->texSpecular->bind();
            }

            glBindVertexArray(command.mesh->vao);
            if (command.mesh->indices.empty()) {
                glDrawArrays(GL_TRIANGLES, 0, command.mesh->vertices.size());
                glBindVertexArray(0);
            }
            else {
                glDrawElements(GL_TRIANGLES, command.mesh->indices.size(), GL_UNSIGNED_INT, 0);
            }
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

    GLuint depthMapFBO;
    GLuint depthMap;

    GLuint depthMapQuadVBO;

    std::vector<RenderCommand> renderCommands;

    DirLight dirLight;

    Ref<Shader> depthShader;
    Ref<Shader> phongShader;
    Ref<Shader> debugDepthShader;
};

#endif //MOTION_EDITING_PHONGRENDERER_H

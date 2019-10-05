//
// Created by lasagnaphil on 19. 5. 7.
//

#ifndef GENGINE_GIZMOSRENDERER_H
#define GENGINE_GIZMOSRENDERER_H

#include "LineMesh.h"
#include "FlyCamera.h"

struct LineRenderCommand {
    Ref<LineMesh> mesh;
    Ref<LineMaterial> material;
    glm::mat4 modelMatrix;
};

struct GizmosRenderer {
    GizmosRenderer(Camera* camera = nullptr) : camera(camera) {}

    void setCamera(Camera* camera) {
        this->camera = camera;
    }

    void init() {

    }

    void queueLine(const LineRenderCommand& command) {
        lineRenderCommands.push_back(command);
    }

    void render() {
        Shaders::line3D->use();
        Shaders::line3D->setCamera(camera);
        Shaders::point->use();
        Shaders::point->setCamera(camera);

        for (auto& command : lineRenderCommands) {
            glBindVertexArray(command.mesh->vao);

            auto& material = command.material;
            if (material->drawLines) {
                Shaders::line3D->use();
                Shaders::line3D->setMat4("model", command.modelMatrix);
                Shaders::line3D->setVec4("color", material->lineColor);
#ifndef __APPLE__
                glLineWidth(material->lineWidth);
#endif
                glDrawArrays(material->lineType, 0, command.mesh->positions.size());
            }
            if (material->drawPoints) {
                Shaders::point->use();
                Shaders::point->setMat4("model", command.modelMatrix);
                Shaders::point->setVec4("color", material->pointColor);
                glPointSize(material->pointSize);
                glDrawArrays(GL_POINTS, 0, command.mesh->positions.size());
            }

            glBindVertexArray(0);
        }
        lineRenderCommands.clear();
    }

private:
    Camera* camera;
    std::vector<LineRenderCommand> lineRenderCommands;
};
#endif //GENGINE_GIZMOSRENDERER_H

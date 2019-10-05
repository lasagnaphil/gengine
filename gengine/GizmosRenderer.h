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

    void render();

private:
    Camera* camera;
    std::vector<LineRenderCommand> lineRenderCommands;

};
#endif //GENGINE_GIZMOSRENDERER_H

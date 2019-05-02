//
// Created by lasagnaphil on 19. 3. 7.
//

#ifndef MOTION_EDITING_LINEMESH_H
#define MOTION_EDITING_LINEMESH_H

#include "Storage.h"
#include <glm/vec2.hpp>
#include <glad/glad.h>

class LineMesh2D {
public:
    LineMesh2D() = default;

    LineMesh2D(std::vector<glm::vec2> positions,
               float height,
               bool drawPoints = false,
               GLenum lineType = GL_LINE_STRIP) :
        positions(std::move(positions)), height(height),
        drawPoints(drawPoints), lineType(lineType) {}

    void init();

    void addVertex(glm::vec2 pos) {
        positions.push_back(pos);
    }

    void removeAtIndex(std::size_t i) {
        positions.erase(positions.begin() + i);
    }

    void subdivisionBSpline(int interpFactor);

    void draw();

private:
    std::vector<glm::vec2> positions;
    float height;

    bool drawPoints = false;
    GLenum lineType = GL_LINE_STRIP;

    GLuint vao;
    GLuint vbo;
};

#endif //MOTION_EDITING_LINEMESH_H

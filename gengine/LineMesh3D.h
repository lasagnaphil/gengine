//
// Created by lasagnaphil on 19. 3. 11.
//

#ifndef MOTION_EDITING_LINEMESH3D_H
#define MOTION_EDITING_LINEMESH3D_H

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glad/glad.h>
#include <span.hpp>

#include "Storage.h"

class LineMesh3D {
public:
    LineMesh3D() = default;

    LineMesh3D(std::vector<glm::vec3> positions) :
        positions(std::move(positions)) {}

    void init();

    void addVertex(glm::vec3 pos) {
        positions.push_back(pos);
    }

    void removeAtIndex(std::size_t i) {
        positions.erase(positions.begin() + i);
    }

    void setPositions(std::vector<glm::vec3> p) {
        positions = std::move(p);
    }

    void subdivisionBSpline(int interpFactor);

    void draw(const glm::mat4& model);

    void updateBuffers();

    void updateAllPoints(nonstd::span<glm::vec3> points);

    void updatePoint(std::size_t idx, const glm::vec3& point);

    std::vector<glm::vec3> positions;

    bool drawLines = true;
    bool drawPoints = false;

    GLenum lineType = GL_LINE_STRIP;
    float lineWidth = 1.0f;
    glm::vec4 lineColor = {1.0f, 0.0f, 0.0f, 1.0f};

    float pointSize = 4.0f;
    glm::vec4 pointColor;

private:
    GLuint vao;
    GLuint vbo;
};

#endif //MOTION_EDITING_LINEMESH3D_H

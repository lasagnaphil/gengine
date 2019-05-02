//
// Created by lasagnaphil on 19. 3. 11.
//

#include "LineMesh3D.h"
#include "Shader.h"

void LineMesh3D::init() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * positions.size(), positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void LineMesh3D::subdivisionBSpline(int interpFactor) {
    std::vector<glm::vec3> prev = positions;
    std::vector<glm::vec3> next;
    int numVertices = positions.size();
    for (int iter = 0; iter < interpFactor; iter++) {
        next.resize(2*numVertices);
        for (int i = 0; i < numVertices; i++) {
            int iprev = (i == 0)? 0 : (i - 1);
            int inext = (i == numVertices - 1)? numVertices - 1 : (i + 1);
            next[2*i] = (prev[iprev] + 6.f * prev[i] + prev[inext]) / 8.f;
            next[2*i + 1] = (prev[i] + prev[inext]) / 2.f;
        }
        numVertices *= 2;
        std::swap(prev, next);
    }
    positions = prev;

}

void LineMesh3D::draw(const glm::mat4& model) {
    glBindVertexArray(vao);

    if (drawLines) {
        Shaders::line3D->use();
        Shaders::line3D->setMat4("model", model);
        Shaders::line3D->setVec4("color", lineColor);
        glLineWidth(lineWidth);
        glDrawArrays(lineType, 0, positions.size());
    }
    if (drawPoints) {
        Shaders::point->use();
        Shaders::point->setMat4("model", model);
        Shaders::point->setVec4("color", pointColor);
        glPointSize(pointSize);
        glDrawArrays(GL_POINTS, 0, positions.size());
    }

    glBindVertexArray(0);
}

void LineMesh3D::updateBuffers() {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * positions.size(), positions.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void LineMesh3D::updateAllPoints(nonstd::span<glm::vec3> points) {
    assert(points.size() == positions.size());
    positions = std::vector<glm::vec3>(points.begin(), points.end());
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * points.size(), points.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void LineMesh3D::updatePoint(std::size_t idx, const glm::vec3& point) {
    positions[idx] = point;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*idx, sizeof(glm::vec3), &positions[idx]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


//
// Created by lasagnaphil on 19. 3. 11.
//

#include "LineMesh2D.h"
#include "Shader.h"

void LineMesh2D::init() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * positions.size(), positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void LineMesh2D::subdivisionBSpline(int interpFactor) {
    std::vector<glm::vec2> prev = positions;
    std::vector<glm::vec2> next;
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

void LineMesh2D::draw() {
    Shader* shader = Shaders::line2D.get();
    shader->use();
    shader->setFloat("height", height);
    shader->setMat4("model", glm::mat4(1.0f));
    shader->setVec4("color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    shader->setFloat("pointSize", 3.0f);
    glBindVertexArray(vao);
    glDrawArrays(GL_LINE_STRIP, 0, positions.size());
    if (drawPoints) {
        glDrawArrays(GL_POINTS, 0, positions.size());
    }
    glBindVertexArray(0);
}

//
// Created by lasagnaphil on 19. 3. 8.
//

#ifndef MOTION_EDITING_UVMESH_H
#define MOTION_EDITING_UVMESH_H

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glad/glad.h>

#include <vector>

#include "Shader.h"
#include "Storage.h"

struct Mesh {
    static float cubeVertices[8*36];
    static float planeVertices[8*6];

    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    Mesh(std::vector<Vertex> vertices = {}, std::vector<uint32_t> indices = {}) :
        vertices(std::move(vertices)), indices(std::move(indices)) {}
    Mesh(Vertex* vertexData, std::size_t vertexCount) :
        vertices(vertexData, vertexData + vertexCount), indices() {}
    Mesh(Vertex* vertexData, std::size_t vertexCount, float* indexData, std::size_t indexCount) :
        vertices(vertexData, vertexData + vertexCount), indices(indexData, indexData + indexCount) {}

    void initVBO() {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Mesh::Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

        if (!indices.empty()) {
            glGenBuffers(1, &ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
        }

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)offsetof(Mesh::Vertex, normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)offsetof(Mesh::Vertex, uv));
        glBindVertexArray(0);
    }

    void updateVBO() {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size(), vertices.data());
    }

    static Ref<Mesh> makeCube(const glm::vec3& scale = {1.0f, 1.0f, 1.0f}) {
        std::vector<Vertex> vertices((Vertex*)cubeVertices, ((Vertex*)cubeVertices) + 36);
        for (int i = 0; i < 36; i++) {
            vertices[i].pos *= scale;
        }
        return Resources::make<Mesh>(vertices);
    };

    static Ref<Mesh> makePlane(float size = 1.0f, float uvSize = 1.0f) {
        std::vector<Vertex> vertices((Vertex*)planeVertices, ((Vertex*)planeVertices) + 6);
        for (int i = 0; i < 6; i++) {
            vertices[i].pos *= size;
            vertices[i].uv *= uvSize;
        }
        return Resources::make<Mesh>(vertices);
    }

    static Ref<Mesh> makeCylinder(unsigned int numQuads, float r, float h) {
        std::vector<Vertex> vertices;
        vertices.reserve(numQuads * 6);
        float delta = glm::two_pi<float>() / numQuads;
        for (unsigned int i = 0; i < numQuads; ++i) {
            float theta = i * delta;
            float thetap = (i + 1) * delta;
            glm::vec3 normal(r * glm::cos(theta), h, -r * glm::sin(theta));
            glm::vec3 normalp(r * glm::cos(thetap), h, -r * glm::sin(thetap));
            vertices.push_back(Vertex{{r * glm::cos(theta), 0, -r * glm::sin(theta)}, normal, {0.0f, 0.0f}});
            vertices.push_back(Vertex{{r * glm::cos(theta), h, -r * glm::sin(theta)}, normal, {0.0f, 1.0f}});
            vertices.push_back(Vertex{{r * glm::cos(thetap), h, -r * glm::sin(thetap)}, normalp, {1.0f, 1.0f}});
            vertices.push_back(Vertex{{r * glm::cos(theta), 0, -r * glm::sin(theta)}, normal, {0.0f, 0.0f}});
            vertices.push_back(Vertex{{r * glm::cos(thetap), h, -r * glm::sin(thetap)}, normalp, {1.0f, 1.0f}});
            vertices.push_back(Vertex{{r * glm::cos(thetap), 0, -r * glm::sin(thetap)}, normalp, {1.0f, 0.0f}});
        }
        return Resources::make<Mesh>(vertices);
    }

    static Ref<Mesh> makeCone(unsigned int numTriangles, float r, float h) {
        std::vector<Vertex> vertices;
        vertices.reserve(numTriangles * 3);
        float delta = glm::two_pi<float>() / numTriangles;
        for (unsigned int i = 0; i < numTriangles; ++i) {
            float theta = i * delta;
            float thetap = (i + 1) * delta;
            float thetam = (theta + thetap) / 2;
            float slopeLen = glm::sqrt(r*r + h*h);
            glm::vec3 normal(glm::cos(theta) * h / slopeLen, r / slopeLen, -glm::sin(theta) * h / slopeLen);
            glm::vec3 normalm(glm::cos(thetam) * h / slopeLen, r / slopeLen, -glm::sin(thetam) * h / slopeLen);
            glm::vec3 normalp(glm::cos(thetap) * h / slopeLen, r / slopeLen, -glm::sin(thetap) * h / slopeLen);
            vertices.push_back(Vertex{{0.0f, h, 0.0f}, normalm, {0.0f, 0.0f}});
            vertices.push_back(Vertex{{r * glm::cos(theta), 0, -r * glm::sin(theta)}, normal, {1.0f, 0.0f}});
            vertices.push_back(Vertex{{r * glm::cos(thetap), 0, -r * glm::sin(thetap)}, normalp, {0.0f, 1.0f}});
        }
        return Resources::make<Mesh>(vertices);
    }
};
#endif //MOTION_EDITING_UVMESH_H

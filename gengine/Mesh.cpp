//
// Created by lasagnaphil on 19. 3. 16.
//

#include "Mesh.h"

float Mesh::cubeVertices[8*36] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
};

float Mesh::planeVertices[8*6] = {
        -0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,

        -0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
};

void Mesh::initVBO() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Mesh::Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    if (!indices.empty()) {
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW);
    }

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)offsetof(Mesh::Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)offsetof(Mesh::Vertex, uv));
    glBindVertexArray(0);
}

void Mesh::updateVBO() {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size(), vertices.data());
}

Ref<Mesh> Mesh::makeCube(const glm::vec3 &scale) {
    std::vector<Vertex> vertices((Vertex*)cubeVertices, ((Vertex*)cubeVertices) + 36);
    for (int i = 0; i < 36; i++) {
        vertices[i].pos *= scale;
    }
    auto mesh = Resources::make<Mesh>(vertices);
    mesh->initVBO();
    return mesh;
}

Ref<Mesh> Mesh::makePlane(float size, float uvSize) {
    std::vector<Vertex> vertices((Vertex*)planeVertices, ((Vertex*)planeVertices) + 6);
    for (int i = 0; i < 6; i++) {
        vertices[i].pos *= size;
        vertices[i].uv *= uvSize;
    }
    auto mesh = Resources::make<Mesh>(vertices);
    mesh->initVBO();
    return mesh;
}

Ref<Mesh> Mesh::makeCylinder(unsigned int numQuads, float r, float h) {
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
    auto mesh = Resources::make<Mesh>(vertices);
    mesh->initVBO();
    return mesh;
}

Ref<Mesh> Mesh::makeCone(unsigned int numTriangles, float r, float h) {
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
    auto mesh = Resources::make<Mesh>(vertices);
    mesh->initVBO();
    return mesh;
}

Ref<Mesh> Mesh::makeSphere(float radius, unsigned int sectorCount, unsigned int stackCount) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    vertices.reserve(stackCount * sectorCount);
    indices.reserve(3 * stackCount * sectorCount);

    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
    float s, t;                                     // vertex texCoord

    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for(int i = 0; i <= stackCount; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for(int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;

            vertices.push_back(Vertex {{x, y, z}, {nx, ny, nz}, {s, t}});
        }
    }

    int k1, k2;
    for(int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if(i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if(i != (stackCount-1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    auto mesh = Resources::make<Mesh>(vertices, indices);
    mesh->initVBO();
    return mesh;
}



//
// Created by lasagnaphil on 19. 12. 17..
//

#include "PhysXDebugRenderer.h"
#include "PhysicsWorld.h"
#include "Shader.h"

#include "shaders/physx_debug.vert.h"
#include "shaders/physx_debug.frag.h"

using namespace physx;

void PhysXDebugRenderer::init(PhysicsWorld& world) {
    world.scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
    // world.scene->setVisualizationParameter(PxVisualizationParameter::eWORLD_AXES, 0.1f);
    // world.scene->setVisualizationParameter(PxVisualizationParameter::eBODY_AXES, 1.0f);
    // world.scene->setVisualizationParameter(PxVisualizationParameter::eBODY_MASS_AXES, 1.0f);
    // world.scene->setVisualizationParameter(PxVisualizationParameter::eBODY_LIN_VELOCITY, 1.0f);
    // world.scene->setVisualizationParameter(PxVisualizationParameter::eCONTACT_POINT, 1.0f);
    // world.scene->setVisualizationParameter(PxVisualizationParameter::eCONTACT_NORMAL, 1.0f);
    // world.scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 1.0f);
    // world.scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_AABBS, 1.0f);
    world.scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
    // world.scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_AXES, 1.0f);
    // world.scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_COMPOUNDS, 1.0f);
    // world.scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_FNORMALS, 1.0f);
    // world.scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_EDGES, 1.0f);
    // world.scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_STATIC, 1.0f);
    // world.scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_DYNAMIC, 1.0f);
    world.scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 0.1f);
    // world.scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.0f);
    // world.scene->setVisualizationParameter(PxVisualizationParameter::eMBP_REGIONS, 1.0f);

    debugShader = Resources::make<Shader>("physx_debug");
    debugShader->compileFromString(physx_debug_vert_shader, physx_debug_frag_shader);

    const PxRenderBuffer &renderBuffer = world.getRenderBuffer();

    lines = {renderBuffer.getLines(), renderBuffer.getNbLines()};
    points = {renderBuffer.getPoints(), renderBuffer.getNbPoints()};
    triangles = {renderBuffer.getTriangles(), renderBuffer.getNbTriangles()};

    glGenVertexArrays(1, &lineVao);
    glBindVertexArray(lineVao);
    glGenBuffers(1, &lineVbo);
    glBindBuffer(GL_ARRAY_BUFFER, lineVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PxDebugLine) * 65536, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 16, (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 16, (void *) 12);
    glBindVertexArray(0);

    glGenVertexArrays(1, &pointVao);
    glBindVertexArray(pointVao);
    glGenBuffers(1, &pointVbo);
    glBindBuffer(GL_ARRAY_BUFFER, pointVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PxDebugPoint) * 65536, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 16, (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 16, (void *) 12);
    glBindVertexArray(0);

    glGenVertexArrays(1, &triangleVao);
    glBindVertexArray(triangleVao);
    glGenBuffers(1, &triangleVbo);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PxDebugTriangle) * 65536, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 16, (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 16, (void *) 12);
    glBindVertexArray(0);
}

void PhysXDebugRenderer::render(PhysicsWorld& world) {

    const PxRenderBuffer &renderBuffer = world.getRenderBuffer();

    lines = {renderBuffer.getLines(), renderBuffer.getNbLines()};
    points = {renderBuffer.getPoints(), renderBuffer.getNbPoints()};
    triangles = {renderBuffer.getTriangles(), renderBuffer.getNbTriangles()};

    debugShader->use();
    debugShader->setCamera(camera);
    debugShader->setMat4("model", glm::mat4(1.0f));

    glBindVertexArray(lineVao);
    glBindBuffer(GL_ARRAY_BUFFER, lineVbo);
    void *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    memcpy(ptr, lines.data(), sizeof(PxDebugLine)*lines.size());
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glDrawArrays(GL_LINES, 0, 2*lines.size());

    glBindVertexArray(pointVao);
    glBindBuffer(GL_ARRAY_BUFFER, pointVbo);
    ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    memcpy(ptr, points.data(), sizeof(PxDebugPoint)*points.size());
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glDrawArrays(GL_POINTS, 0, points.size());

    glBindVertexArray(triangleVao);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVbo);
    ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    memcpy(ptr, triangles.data(), sizeof(PxDebugTriangle)*triangles.size());
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_TRIANGLES, 0, 3*triangles.size());
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindVertexArray(0);
}

//
// Created by lasagnaphil on 19. 5. 13.
//

#ifndef PHYSICS_BENCHMARKS_PHYSXDEBUGRENDERER_H
#define PHYSICS_BENCHMARKS_PHYSXDEBUGRENDERER_H

#ifndef NDEBUG
#if !defined _DEBUG
#define _DEBUG 1
#endif
#endif

#include <glad/glad.h>
#include <span.hpp>
#include <common/PxRenderBuffer.h>
#include "gengine/Camera.h"
#include "gengine/Shader.h"
#include "PhysicsWorld.h"

struct PhysXDebugRenderer {
    GLuint lineVao;
    GLuint lineVbo;
    GLuint pointVao;
    GLuint pointVbo;
    GLuint triangleVao;
    GLuint triangleVbo;

    nonstd::span<const physx::PxDebugLine> lines;
    nonstd::span<const physx::PxDebugPoint> points;
    nonstd::span<const physx::PxDebugTriangle> triangles;

    Camera *camera;

    Ref<Shader> debugShader = {};

    PhysXDebugRenderer(Camera *camera = nullptr) : camera(camera) {}

    void setCamera(Camera* camera) {
        this->camera = camera;
    }

    void init(PhysicsWorld &world);

    void render(PhysicsWorld& world);
};

#endif //PHYSICS_BENCHMARKS_PHYSXDEBUGRENDERER_H


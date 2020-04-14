//
// Created by lasagnaphil on 19. 5. 7.
//

#ifndef PHYSICS_BENCHMARKS_PHYSICSWORLD_H
#define PHYSICS_BENCHMARKS_PHYSICSWORLD_H

#ifndef NDEBUG
#if !defined _DEBUG
#define _DEBUG 1
#endif
#endif

#include <PxPhysicsAPI.h>
#include <pvd/PxPvd.h>
#include <extensions/PxDefaultAllocator.h>
#include <extensions/PxDefaultErrorCallback.h>

#include "PhysXGLM.h"

using namespace physx;

struct PhysicsWorld {
    static PxFoundation* foundation;
    static int worldCount;

    PxDefaultAllocator allocator = {};
    PxDefaultErrorCallback errorCallback = {};

    PhysicsWorld() = default;
    ~PhysicsWorld() {
        if (foundation) release();
    }

    void init(uint32_t numThread = 1, bool enableGpu = false);

    void simulate(float dt);

    void release();

    glm::vec3 getGravity() {
        return PxToGLM(scene->getGravity());
    }

    void setGravity(glm::vec3 g) {
        scene->setGravity(GLMToPx(g));
    }

    glm::vec3 getGroundPos() {
        return PxToGLM(groundPlane->getGlobalPose().p);
    }

    void setGroundPos(glm::vec3 pos) {
        PxTransform t = groundPlane->getGlobalPose();
        t.p = GLMToPx(pos);
        groundPlane->setGlobalPose(t);
    }

    const PxRenderBuffer& getRenderBuffer() const {
        return scene->getRenderBuffer();
    }

    PxPhysics* physics = nullptr;
    PxCudaContextManager* cudaContextManager;
    PxCooking* cooking = nullptr;
    PxCpuDispatcher* cpuDispatcher = nullptr;
    PxScene* scene = nullptr;

    PxMaterial* defaultMaterial;
    PxRigidStatic* groundPlane;
};

#endif //PHYSICS_BENCHMARKS_PHYSICSWORLD_H

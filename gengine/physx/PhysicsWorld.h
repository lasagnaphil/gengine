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
    void init(PxFoundation* foundation, uint32_t numThread = 1, bool enableGpu = false);

    bool advance(float dt);

    bool fetchResults();

    void release();

    glm::vec3 getGravity() {
        return PxToGLM(scene->getGravity());
    }

    void setGravity(glm::vec3 g) {
        scene->setGravity(GLMToPx(g));
    }

    const PxRenderBuffer& getRenderBuffer() const {
        return scene->getRenderBuffer();
    }

    PxFoundation* foundation;
    PxPhysics* physics;
    // PxCudaContextManager* cudaContextManager;
    PxCooking* cooking;
    PxCpuDispatcher* cpuDispatcher;
    PxScene* scene;
    PxMaterial* defaultMaterial;
};

#endif //PHYSICS_BENCHMARKS_PHYSICSWORLD_H

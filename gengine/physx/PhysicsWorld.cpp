//
// Created by lasagnaphil on 19. 5. 7.
//

#include "PhysicsWorld.h"
#include <iostream>

class GpuLoadHook : public PxGpuLoadHook {
    virtual const char* getPhysXGpuDllName() const {
        return "/home/lasagnaphil/packages/PhysX/physx/bin/linux.clang/release/libPhysXGpu_64.so";
    }
} gGpuLoadHook;

PxFoundation* PhysicsWorld::foundation = nullptr;
int PhysicsWorld::worldCount = 0;

void PhysicsWorld::init(uint32_t numThreads, bool enableGpu) {
    // Singleton pattern: only create foundation if it doesn't exist yet
    if (!foundation) {
        foundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errorCallback);
        if (!foundation) {
            std::cerr << "PxCreateFoundation failed!" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    PxTolerancesScale scale;
    scale.length = 1.0f;
    scale.speed = 9.81f;

    physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation,
                              scale, true, nullptr);
    if (!physics) {
        std::cerr << "PxCreatePhysics failed!" << std::endl;
        exit(EXIT_FAILURE);
    }

    PxCookingParams params(scale);
    cooking = PxCreateCooking(PX_PHYSICS_VERSION, *foundation, params);
    if (!cooking) {
        std::cerr << "PxCreateCooking failed!" << std::endl;
        exit(EXIT_FAILURE);
    }


    PxSceneDesc sceneDesc(physics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    cpuDispatcher = PxDefaultCpuDispatcherCreate(numThreads);
    sceneDesc.cpuDispatcher = cpuDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    // sceneDesc.solverType = PxSolverType::eTGS;

    // enable CUDA
    if (enableGpu) {
        PxSetPhysXGpuLoadHook(&gGpuLoadHook);

        PxCudaContextManagerDesc cudaContextManagerDesc;
        auto cudaContextManager = PxCreateCudaContextManager(*foundation, cudaContextManagerDesc, PxGetProfilerCallback());
        sceneDesc.cudaContextManager = cudaContextManager;
        sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
        sceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;
    }

    scene = physics->createScene(sceneDesc);
    defaultMaterial = physics->createMaterial(0.5f, 0.5f, 0.6f);

    // create ground
    groundPlane = PxCreatePlane(*physics, PxPlane(0,1,0,0), *physics->createMaterial(1.0f, 0.5f, 0.05f));
    scene->addActor(*groundPlane);

    worldCount++;
}

bool PhysicsWorld::advance(float dt, float stepSize) {
    static float time = 0.0f;
    time += dt;
    if (time < stepSize) {
        return false;
    }
    time -= stepSize;
    scene->simulate(stepSize);
    return true;
}

bool PhysicsWorld::fetchResults() {
    return scene->fetchResults(true);
}

void PhysicsWorld::simulate(float dt) {
    scene->simulate(dt);
    scene->fetchResults(true);
}

void PhysicsWorld::release() {
    scene->release();
    cooking->release();
    physics->release();

    worldCount--;
    if (worldCount == 0) {
        foundation->release();
    }
}

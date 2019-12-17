//
// Created by lasagnaphil on 19. 5. 7.
//

#include "PhysicsWorld.h"
#include <iostream>

void PhysicsWorld::init(PxFoundation* foundation, uint32_t numThreads = 16) {
    this->foundation = foundation;

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

    // enable CUDA

    /*
    PxCudaContextManagerDesc cudaContextManagerDesc;
    cudaContextManager = PxCreateCudaContextManager(*foundation, cudaContextManagerDesc, PxGetProfilerCallback());
    sceneDesc.cudaContextManager = cudaContextManager;
    sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
    sceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;
     */

    scene = physics->createScene(sceneDesc);
    defaultMaterial = physics->createMaterial(0.5f, 0.5f, 0.6f);

    // create ground
    PxRigidStatic* groundPlane = PxCreatePlane(*physics, PxPlane(0,1,0,0), *defaultMaterial);
    scene->addActor(*groundPlane);
}

bool PhysicsWorld::advance(float dt) {
    static float time = 0.0f;
    constexpr float stepSize = 1.0f / 60.0f;
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

void PhysicsWorld::release() {
    cooking->release();
    physics->release();
}

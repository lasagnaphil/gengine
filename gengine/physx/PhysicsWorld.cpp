//
// Created by lasagnaphil on 19. 5. 7.
//

#include "PhysicsWorld.h"
#include <iostream>

PxFoundation* PhysicsWorld::foundation = nullptr;
int PhysicsWorld::worldCount = 0;

void PhysicsWorld::init(uint32_t numThreads, bool enableGpu, PxSimulationFilterShader filterShader) {
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
    sceneDesc.filterShader = filterShader;
    sceneDesc.solverType = PxSolverType::ePGS;

    // enable CUDA
    if (enableGpu) {
        PxCudaContextManagerDesc cudaContextManagerDesc;
        cudaContextManager = PxCreateCudaContextManager(*foundation, cudaContextManagerDesc, PxGetProfilerCallback());
        if (cudaContextManager) {
            if (cudaContextManager->contextIsValid()) {
                sceneDesc.cudaContextManager = cudaContextManager;
                sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
                sceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;
                sceneDesc.gpuMaxNumPartitions = 32;
            }
            else {
                std::cerr << "Failed to create PxCudaContextManager!" << std::endl;
                cudaContextManager->release();
                cudaContextManager = nullptr;
            }
        }
    }

    scene = physics->createScene(sceneDesc);
    defaultMaterial = physics->createMaterial(1.0f, 1.0f, 0.0f);

    // create ground
    groundPlane = PxCreatePlane(*physics, PxPlane(0,1,0,0), *physics->createMaterial(1.0f, 1.0f, 0.0f));
    scene->addActor(*groundPlane);

    worldCount++;
    std::cout << "World " << worldCount << " created!" << std::endl;
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

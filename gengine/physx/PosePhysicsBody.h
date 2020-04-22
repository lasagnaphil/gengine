//
// Created by lasagnaphil on 19. 11. 21..
//

#ifndef ANIMATION_PROJECT_POSEPHYSICSBODY_H
#define ANIMATION_PROJECT_POSEPHYSICSBODY_H

#include "anim/PoseTree.h"
#include "glmx/pose.h"
#include "PosePhysicsBodySkel.h"
#include "PhysicsBody.h"

struct PosePhysicsBody {
    physx::PxArticulationReducedCoordinate* articulation;

    PxAggregate *aggregate;
    PxArticulationCache* cache;

    std::vector<uint32_t> dofStarts;
    std::unordered_map<uint32_t, PxArticulationLink*> nodeToLink;
    uint32_t dofCount = 0;

    void init(PhysicsWorld& world, const PoseTree& poseTree, const PosePhysicsBodySkel& skel);

    void setPose(const glmx::pose& pose, const PoseTree& poseTree);

    void getPose(glmx::pose& pose, const PoseTree& poseTree);

    void setPoseVelocityFromTwoPoses(const glmx::pose& p1, const glmx::pose& p2, float dt);

    void setRoot(const glmx::transform& rootTrans);

    void putToSleep() {
        articulation->putToSleep();
    }

    void wakeUp() {
        articulation->wakeUp();
    }

    void applyImpulseTorqueAtRoot(glm::vec3 impulse) {
        nodeToLink[0]->addTorque(GLMToPx(impulse), PxForceMode::eIMPULSE, true);
    }

    void applyImpulseAtRoot(glm::vec3 impulse) {
        nodeToLink[0]->addForce(GLMToPx(impulse), PxForceMode::eIMPULSE, true);
    }

    void applyVelocityChangeAtRoot(glm::vec3 velChange) {
        nodeToLink[0]->addForce(GLMToPx(velChange), PxForceMode::eVELOCITY_CHANGE, true);
    }

    void renderImGui();
};
#endif //ANIMATION_PROJECT_POSEPHYSICSBODY_H

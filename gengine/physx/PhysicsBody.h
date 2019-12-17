//
// Created by lasagnaphil on 19. 12. 11..
//

#ifndef ANIMATION_PROJECT_PHYSICSBODY_H
#define ANIMATION_PROJECT_PHYSICSBODY_H

#include <Defer.h>
#include "PhysXGLM.h"
#include "PxRigidDynamic.h"
#include "PhysicsBody.h"
#include "PhysicsWorld.h"
#include "Mesh.h"

struct PhysicsBody {
    physx::PxRigidDynamic* body = nullptr;

    static PhysicsBody box(PhysicsWorld& world,
                           PxMaterial* mat,
                           float density,
                           glm::vec3 pos = {},
                           glm::quat rot = glm::identity<glm::quat>(),
                           glm::vec3 size = glm::vec3(1.0f));

    static PhysicsBody ourBox(PhysicsWorld& world,
                              float x, float y, float z, float thickness,
                              PxMaterial* mat,
                              glm::vec3 pos = glm::vec3(0.0f),
                              glm::quat rot = glm::identity<glm::quat>(),
                              glm::vec3 scale = glm::vec3(1.0f));

    static PhysicsBody sphere(PhysicsWorld& world,
                              PxMaterial* mat,
                              float density,
                              glm::vec3 pos = {},
                              float size = 1.0f);

    static std::optional<PhysicsBody> fromMesh(const PhysicsWorld& world,
                                               const MeshCollider& mesh,
                                               PxMaterial* mat,
                                               glm::vec3 pos = glm::vec3(0.0f),
                                               glm::quat rot = glm::identity<glm::quat>(),
                                               glm::vec3 scale = glm::vec3(1.0f));

    glm::vec3 getPosition() {
        return PxToGLM(body->getGlobalPose().p);
    }

    void setPosition(glm::vec3 v) {
        body->setGlobalPose(PxTransform(GLMToPx(v), body->getGlobalPose().q));
    }

    glm::quat getRotation() {
        return PxToGLM(body->getGlobalPose().q);
    }

    void setRotation(glm::quat q) {
        body->setGlobalPose(PxTransform(body->getGlobalPose().p, GLMToPx(q)));
    }

    void setTransform(const glmx::transform& t) {
        body->setGlobalPose(GLMToPx(t));
    }

    glmx::transform getTransform() {
        return PxToGLM(body->getGlobalPose());
    }

    glm::vec3 getLinearVelocity() {
        return PxToGLM(body->getLinearVelocity());
    }

    void setLinearVelocity(glm::vec3 v, bool autowake = true) {
        return body->setLinearVelocity(GLMToPx(v), autowake);
    }

    glm::vec3 getAngularVelocity() {
        return PxToGLM(body->getAngularVelocity());
    }

    void setAngularVelocity(glm::vec3 w, bool autowake = true) {
        return body->setAngularVelocity(GLMToPx(w), autowake);
    }

    void setKinematic(bool enable) {
        body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, enable);
    }
};

#endif //ANIMATION_PROJECT_PHYSICSBODY_H

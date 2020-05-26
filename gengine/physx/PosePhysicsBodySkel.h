//
// Created by lasagnaphil on 19. 12. 5..
//

#ifndef ANIMATION_PROJECT_POSEPHYSICSBODYSKEL_H
#define ANIMATION_PROJECT_POSEPHYSICSBODYSKEL_H

#include <tinyxml2.h>

#include <unordered_map>
#include "gengine/PoseRenderBody.h"

static glm::vec3 stringToVec3(const char* str) {
    glm::vec3 v;
    std::sscanf(str, "%f %f %f", &v.x, &v.y, &v.z);
    return v;
}

static glm::mat3 stringToMat3(const char* str) {
    glm::mat3 m;
    std::sscanf(str, "%f %f %f %f %f %f %f %f %f",
                &m[0][0], &m[0][1], &m[0][2],
                &m[1][0], &m[1][1], &m[1][2],
                &m[2][0], &m[2][1], &m[2][2]);
    return m;
}

struct PosePhysicsBodySkel {
    enum class ShapeType {
        None, Capsule, Cylinder, Sphere, Box
    };
    struct CapsuleShape {
        glm::vec3 direction;
        glm::vec3 offset;
        float radius;
        float height;
    };
    struct CylinderShape {
        glm::vec3 direction;
        glm::vec3 offset;
        float radius;
        float height;
    };
    struct SphereShape {
        glm::vec3 offset;
        float radius;
    };
    struct BoxShape {
        glm::vec3 size;
        glm::vec3 offset;
    };
    struct Shape {
        ShapeType type;
        union {
            CapsuleShape capsule;
            CylinderShape cylinder;
            SphereShape sphere;
            BoxShape box;
        };
    };
    struct Joint {
        std::string name;
        uint32_t bvhIdx;
        uint32_t parentIdx;
        std::vector<uint32_t> childIdx;
        glm::vec3 size;
        float mass;
        glm::vec3 xdir;
        glm::vec3 bodyTrans;
        glm::vec3 jointTrans;
        Shape shape;
    };

    std::string skeletonName;
    std::vector<Joint> joints;
    std::unordered_map<std::string, uint32_t> jointNameMapping;
    std::unordered_map<uint32_t, uint32_t> bvhMapping;

    const Joint& getJoint(uint32_t idx) const {
        return joints[idx];
    }

    Joint& getJoint(uint32_t idx) {
        return joints[idx];
    }

    static PosePhysicsBodySkel fromFile(
            const std::string& filename, const PoseTree& poseTree);
};


PoseRenderBodyPBR createFromSkel(const PoseTree& poseTree, PosePhysicsBodySkel& skel,
                                        const std::vector<Ref<PBRMaterial>>& materials);


#endif //ANIMATION_PROJECT_POSEPHYSICSBODYSKEL_H

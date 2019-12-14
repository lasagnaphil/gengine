//
// Created by lasagnaphil on 19. 10. 1..
//

#ifndef GENGINE_INVERSEKINEMATICS_H
#define GENGINE_INVERSEKINEMATICS_H

#include <glmx/transform.h>
#include <glmx/eigen.h>
#include <glmx/euler.h>

#include "glmx/pose.h"
#include "MotionClipData.h"

glmx::transform calcFK(const PoseTree& poseTree, const glmx::pose& pose, uint32_t mIdx);

std::vector<glmx::transform> calcFK(const PoseTree& poseTree, const glmx::pose& pose);

enum class LeastSquareMethod {
    SVD, QR, Normal
};

void solveIKSimple(const PoseTree& poseTree, glmx::pose& pose, uint32_t mIdx,
                   nonstd::span<uint32_t> relevantJoints,
                   nonstd::span<float> jointStiffness,
                   glm::vec3 mPos,
                   LeastSquareMethod lsqMethod = LeastSquareMethod::SVD);

void solveIKSimple(const PoseTree& poseTree, glmx::pose& pose, uint32_t mIdx,
                   nonstd::span<uint32_t> relevantJoints,
                   nonstd::span<float> jointStiffness,
                   glmx::transform mT,
                   LeastSquareMethod lsqMethod = LeastSquareMethod::SVD);

// 2-joint IK: http://theorangeduck.com/page/simple-two-joint
void solveTwoJointIK(
        glm::vec3 a, glm::vec3 b, glm::vec3 c,
        glm::vec3 target,
        glm::quat a_gr, glm::quat b_gr, glm::quat& a_lr, glm::quat& b_lr, float eps = 0.01f);

void solveTwoJointIK(const PoseTree& poseTree, glmx::pose& pose,
        uint32_t aIdx, uint32_t bIdx, uint32_t cIdx, glm::vec3 target, float eps = 0.01f);


struct IKProblem {
    glm::vec3 targetPos;
    glm::quat targetRot;
    uint32_t targetIdx;

    float alpha_targetPos = 1.0f;
    float alpha_targetRot = 1.0f;
    float alpha_poseDiff = 0.0f;

    nonstd::span<uint32_t> relevantJoints;

    struct JointLimit {
        float minX, maxX;
        float minY, maxY;
        float minZ, maxZ;
        bool enabled;
    };
    nonstd::span<JointLimit> jointLimits;
    nonstd::span<float> jointStiffness;

};

void solveIK(const PoseTree& poseTree, glmx::pose& pose, const IKProblem& ik);

#endif //GENGINE_INVERSEKINEMATICS_H

//
// Created by lasagnaphil on 19. 10. 1..
//

#ifndef GENGINE_INVERSEKINEMATICS_H
#define GENGINE_INVERSEKINEMATICS_H

#include <glmx/transform.h>
#include <glmx/eigen.h>
#include <glmx/euler.h>

#include "Pose.h"
#include "MotionClipData.h"

glmx::transform calcFK(const PoseTree& poseTree, const Pose& pose, uint32_t mIdx);

std::vector<glmx::transform> calcFK(const PoseTree& poseTree, const Pose& pose);

enum class LeastSquareMethod {
    SVD, QR, Normal
};

void solveIKSimple(const PoseTree& poseTree, Pose& pose, uint32_t mIdx,
                   nonstd::span<uint32_t> relevantJoints, glm::vec3 mPos,
                   LeastSquareMethod lsqMethod = LeastSquareMethod::SVD);

void solveIKSimple(const PoseTree& poseTree, Pose& pose, uint32_t mIdx,
                   nonstd::span<uint32_t> relevantJoints, glmx::transform mT,
                   LeastSquareMethod lsqMethod = LeastSquareMethod::SVD);

struct IKProblem {
    glm::vec3 targetPos;
    glm::quat targetRot;
    uint32_t targetIdx;

    float alpha_targetPos = 1.0f;
    float alpha_targetRot = 1.0f;
    float alpha_poseDiff = 0.0f;

    // joint limits are ignored when x <= 0.0f
    nonstd::span<uint32_t> relevantJoints;

    struct JointLimit {
        float minX, maxX;
        float minY, maxY;
        float minZ, maxZ;
    };
    nonstd::span<JointLimit> jointLimits;

};

void solveIK(const PoseTree& poseTree, Pose& pose, const IKProblem& ik);

#endif //GENGINE_INVERSEKINEMATICS_H

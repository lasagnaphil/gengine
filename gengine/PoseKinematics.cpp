//
// Created by lasagnaphil on 19. 10. 1..
//

#include "PoseKinematics.h"

#include "Eigen/Dense"
#include "Eigen/Householder"

Eigen::MatrixXf calcEulerJacobian(
        const PoseTree& poseTree, Pose& pose, uint32_t mIdx, nonstd::span<uint32_t> relevantJoints) {

    Eigen::MatrixXf J(3, 3 * relevantJoints.size());

    glmx::transform mT = calcFK(poseTree, pose, mIdx);
    int c = 0;
    for (uint32_t i : relevantJoints) {
        glmx::transform axisT = calcFK(poseTree, pose, i);
        glm::vec3 v = mT.v - axisT.v;

        glm::vec3 wx = axisT.q * glm::vec3{1, 0, 0};
        J.col(3*c) = GLMToEigen(glm::cross(wx, v));
        glm::vec3 wy = axisT.q * glm::vec3{0, 1, 0};
        J.col(3*c + 1) = GLMToEigen(glm::cross(wy, v));
        glm::vec3 wz = axisT.q * glm::vec3{0, 0, 1};
        J.col(3*c + 2) = GLMToEigen(glm::cross(wz, v));

        c++;
    }

    return J;
}

glmx::transform calcFK(const PoseTree &poseTree, const Pose &pose, uint32_t mIdx) {
    uint32_t i = mIdx;
    if (poseTree[i].isEndSite) {
        i = poseTree[i].parent;
    }

    glmx::transform t(glm::vec3(0.0f), glm::identity<glm::quat>());

    while (true) {
        auto& node = poseTree[i];
        if (poseTree[i].isEndSite) {
            t = glmx::transform(node.offset) * t;
        }
        else {
            t = glmx::transform(node.offset, pose.q[i]) * t;
        }
        if (i == 0) {
            t = glmx::transform(pose.v) * t;
            break;
        }
        else {
            i = node.parent;
        }
    }
    return t;
}

void
solveIK(const PoseTree &poseTree, Pose &pose, uint32_t mIdx, nonstd::span <uint32_t> relevantJoints, glm::vec3 mPos) {
    PoseEuler poseEuler = toEuler(pose, EulOrdXYZs);
    Eigen::MatrixXf J = calcEulerJacobian(poseTree, pose, mIdx, relevantJoints);
    Eigen::Vector3f dp = GLMToEigen(mPos - calcFK(poseTree, pose, mIdx).v);
    Eigen::VectorXf dq = J.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(dp);
    // Eigen::VectorXf dq = (J.transpose() * J).ldlt().solve(J.transpose() * dp);
    // Eigen::VectorXf dq = J.transpose() * dp;
    int c = 0;
    for (uint32_t i : relevantJoints) {
        assert(i < poseEuler.eulerAngles.size());
        poseEuler.eulerAngles[i].x += dq[3*c];
        poseEuler.eulerAngles[i].y += dq[3*c+1];
        poseEuler.eulerAngles[i].z += dq[3*c+2];
        c++;
    }
    pose = toQuat(poseEuler);
}

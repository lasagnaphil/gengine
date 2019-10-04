//
// Created by lasagnaphil on 19. 10. 1..
//

#include "PoseKinematics.h"

#include "Eigen/Dense"
#include "Eigen/Householder"

glm::vec3 quatToTwist(glm::quat q) {
    float im_size = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z);
    glm::vec3 twist = glm::vec3(q.x, q.y, q.z);
    twist *= (2 * atan2f(im_size, q.w) / im_size);
    return twist;
}

Eigen::MatrixXf calcEulerJacobian(
        const PoseTree& poseTree, Pose& pose, uint32_t mIdx, nonstd::span<uint32_t> relevantJoints) {

    Eigen::MatrixXf J(3, 3 * relevantJoints.size());

    std::vector<glmx::transform> Ts = calcFK(poseTree, pose);
    int c = 0;
    for (uint32_t i : relevantJoints) {
        glmx::transform axisT = Ts[i];
        glm::vec3 v = Ts[mIdx].v - axisT.v;

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

/*
Eigen::MatrixXf calcTwistJacobian(
        const PoseTree& poseTree, Pose& pose, uint32_t mIdx, uint32_t rIdx) {

    std::vector<glmx::transform> Ts = calcFK(poseTree, pose);

    int c = 1;
    int i = mIdx;
    while (i == 0) {
        i = poseTree[i].parent;
        c++;
    }
    Eigen::MatrixXf J(3, c + 1);

    i = mIdx;
    c = 0;
    while (true) {
        auto& node = poseTree[i];
        glm::quat parentQ = Ts[node.parent].q;
        glmx::transform t = glmx::transform(parentQ) * Ts[i] * glmx::transform(glm::conjugate(parentQ));
        J.col(3*c) = GLMToEigen(quatToTwist(t.q));
        if (i == 0) {
            break;
        }
        else {
            i = node.parent;
        }
    }
}
 */

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

std::vector<glmx::transform> calcFK(const PoseTree &poseTree, const Pose &pose) {
    std::vector<glmx::transform> transforms(poseTree.numNodes);
    std::stack<std::tuple<uint32_t, uint32_t>> recursionStack;

    transforms[0] = glmx::transform {pose.v, glm::identity<glm::quat>()};
    recursionStack.push({0, 0});

    while (!recursionStack.empty()) {
        auto [idx, parentIdx] = recursionStack.top();
        recursionStack.pop();

        auto& node = poseTree[idx];
        if (poseTree[idx].isEndSite) {
            transforms[idx] = transforms[parentIdx] * glmx::transform(node.offset);
        }
        else {
            transforms[idx] = transforms[parentIdx] * glmx::transform(node.offset, pose.q[idx]);
        }

        for (uint32_t childIdx : node.childJoints) {
            recursionStack.push({childIdx, idx});
        }
    }

    return transforms;
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

void solveIK(const PoseTree& poseTree, Pose& pose, uint32_t mIdx, nonstd::span<uint32_t> relevantJoints,
        std::function<float(const PoseEuler&)> costFunction) {

    PoseEuler poseEuler = toEuler(pose, EulOrdXYZs);
    std::vector<glmx::transform> transforms = calcFK(poseTree, pose);
    float epsilon = 1e-6;
    float alpha = 1000;
    const int numIters = 10000;
    float cost;

    // Gradient descent on cost function!
    for (int iter = 0; iter < numIters; iter++) {
        cost = costFunction(poseEuler);
        if (iter % 100 == 0) {
            printf("Iteration %d: cost = %f\n", iter, cost);
        }
        if (cost < 1e-4) break;
        Eigen::VectorXf grad_cost(3*relevantJoints.size());
        int c = 0;
        for (uint32_t i : relevantJoints) {
            for (int d = 0; d < 3; d++) {
                poseEuler.eulerAngles[i][d] += epsilon;
                grad_cost[3 * c + d] = costFunction(poseEuler) - cost;
                poseEuler.eulerAngles[i][d] -= epsilon;
            }
            c++;
        }

        c = 0;
        for (uint32_t i : relevantJoints) {
            for (int d = 0; d < 3; d++) {
                poseEuler.eulerAngles[i][d] -= alpha * grad_cost(3 * c + d);
            }
            c++;
        }
    }

    pose = toQuat(poseEuler);
}


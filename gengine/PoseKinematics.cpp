//
// Created by lasagnaphil on 19. 10. 1..
//

#include "PoseKinematics.h"
#include "Pose.h"
#include <glmx/eigen.h>

#include "Eigen/Dense"
#include "Eigen/Householder"

using namespace Eigen;

glm::vec3 quatToTwist(glm::quat q) {
    float im_size = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z);
    glm::vec3 twist = glm::vec3(q.x, q.y, q.z);
    twist *= (2 * atan2f(im_size, q.w) / im_size);
    return twist;
}

MatrixXf calcEulerJacobian(const PoseTree& poseTree, nonstd::span<glmx::transform> Ts,
        uint32_t mIdx, nonstd::span<uint32_t> relevantJoints) {

    MatrixXf J(6, 3 * relevantJoints.size());

    int c = 0;
    for (uint32_t i : relevantJoints) {
        glmx::transform axisT = Ts[i];
        glm::vec3 v = Ts[mIdx].v - axisT.v;

        glm::vec3 wx = axisT.q * glm::vec3{1, 0, 0};
        J.block<3,1>(0, 3*c+0) = glmx::toEigen(glm::cross(wx, v));
        J.block<3,1>(3, 3*c+0) = glmx::toEigen(wx);
        glm::vec3 wy = axisT.q * glm::vec3{0, 1, 0};
        J.block<3,1>(0, 3*c+1) = glmx::toEigen(glm::cross(wy, v));
        J.block<3,1>(3, 3*c+1) = glmx::toEigen(wy);
        glm::vec3 wz = axisT.q * glm::vec3{0, 0, 1};
        J.block<3,1>(0, 3*c+2) = glmx::toEigen(glm::cross(wz, v));
        J.block<3,1>(3, 3*c+2) = glmx::toEigen(wz);

        c++;
    }

    return J;
}

MatrixXf calcEulerJacobian(
        const PoseTree& poseTree, Pose& pose, uint32_t mIdx, nonstd::span<uint32_t> relevantJoints) {

    std::vector<glmx::transform> Ts = calcFK(poseTree, pose);
    return calcEulerJacobian(poseTree, Ts, mIdx, relevantJoints);
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

std::vector<glmx::transform> calcFK(const PoseTree& poseTree, const Pose& pose) {
    std::vector<glmx::transform> transforms(poseTree.numNodes);
    std::stack<std::tuple<uint32_t, uint32_t>> recursionStack;

    transforms[0] = glmx::transform{pose.v, glm::identity<glm::quat>()};
    recursionStack.push({0, 0});

    while (!recursionStack.empty()) {
        auto[idx, parentIdx] = recursionStack.top();
        recursionStack.pop();

        auto& node = poseTree[idx];
        if (poseTree[idx].isEndSite) {
            transforms[idx] = transforms[parentIdx] * glmx::transform(node.offset);
        } else {
            transforms[idx] = transforms[parentIdx] * glmx::transform(node.offset, pose.q[idx]);
        }

        for (uint32_t childIdx : node.childJoints) {
            recursionStack.push({childIdx, idx});
        }
    }

    return transforms;
}

std::vector<glmx::transform> calcFK(const PoseTree& poseTree, glm::vec3 rootPos, const VectorXf& poseEuler) {
    std::vector<glmx::transform> transforms(poseTree.numNodes);
    std::stack<std::tuple<uint32_t, uint32_t>> recursionStack;

    transforms[0] = glmx::transform{rootPos, glm::identity<glm::quat>()};
    recursionStack.push({0, 0});

    while (!recursionStack.empty()) {
        auto[idx, parentIdx] = recursionStack.top();
        recursionStack.pop();

        auto& node = poseTree[idx];
        if (poseTree[idx].isEndSite) {
            transforms[idx] = transforms[parentIdx] * glmx::transform(node.offset);
        } else {
            glm::quat q = glmx::eulerToQuat({poseEuler[3*idx], poseEuler[3*idx+1], poseEuler[3*idx+2]}, EulOrdXYZs);
            transforms[idx] = transforms[parentIdx] * glmx::transform(node.offset, q);
        }

        for (uint32_t childIdx : node.childJoints) {
            recursionStack.push({childIdx, idx});
        }
    }

    return transforms;
}

using Vector6f = Matrix<float, 6, -1>;

namespace glmx {
    glm::vec3 log(glm::quat q) {
        q = glm::normalize(q);
        float a = sqrtf(1 - q.w*q.w);
        if (a <= glm::epsilon<float>()) {
            return glm::vec3 {};
        }
        return 2.0f * atan2f(a, q.w) / a * glm::vec3(q.x, q.y, q.z);
    }
    glm::vec3 logdiff(glm::quat q1, glm::quat q2) {
        return glmx::log(q2 * glm::conjugate(q1));
    }
}

void
solveIKSimple(const PoseTree &poseTree, Pose &pose, uint32_t mIdx,
              nonstd::span <uint32_t> relevantJoints, glmx::transform mT, LeastSquareMethod lsqMethod) {
    PoseEuler poseEuler = toEuler(pose, EulOrdXYZs);
    MatrixXf J = calcEulerJacobian(poseTree, pose, mIdx, relevantJoints);
    glmx::transform T = calcFK(poseTree, pose, mIdx);
    VectorXf dp(6);
    dp << glmx::toEigen(mT.v - T.v), glmx::toEigen(glmx::logdiff(mT.q, T.q));
    VectorXf dq;
    switch (lsqMethod) {
        case LeastSquareMethod::SVD:
            dq = J.bdcSvd(ComputeThinU | ComputeThinV).solve(dp);
            break;
        case LeastSquareMethod::QR:
            dq = J.colPivHouseholderQr().solve(dp);
            break;
        case LeastSquareMethod::Normal:
            dq = (J.transpose() * J).ldlt().solve(J.transpose() * dp);
            break;
    }
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
void
solveIKSimple(const PoseTree &poseTree, Pose &pose, uint32_t mIdx,
              nonstd::span <uint32_t> relevantJoints, glm::vec3 mPos, LeastSquareMethod lsqMethod) {
    PoseEuler poseEuler = toEuler(pose, EulOrdXYZs);
    MatrixXf J = calcEulerJacobian(poseTree, pose, mIdx, relevantJoints)
            .block(0, 0, 3, 3*relevantJoints.size());
    glmx::transform T = calcFK(poseTree, pose, mIdx);
    Vector3f dp = glmx::toEigen(mPos - T.v);
    VectorXf dq;
    switch (lsqMethod) {
        case LeastSquareMethod::SVD:
            dq = J.bdcSvd(ComputeThinU | ComputeThinV).solve(dp);
            break;
        case LeastSquareMethod::QR:
            dq = J.colPivHouseholderQr().solve(dp);
            break;
        case LeastSquareMethod::Normal:
            dq = (J.transpose() * J).ldlt().solve(J.transpose() * dp);
            break;
    }
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

float quatDistance(glm::quat q1, glm::quat q2) {
    float dot = q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
    return glm::acos(2*dot*dot - 1);
}

float totalCost(const IKProblem& ik, glmx::transform endEffectorT,
        const VectorXf& pose, const VectorXf& origPose) {

    float cost = 0.5f * ik.alpha_targetPos * glm::length2(endEffectorT.v - ik.targetPos);
    cost += 0.5f * ik.alpha_targetRot * quatDistance(endEffectorT.q, ik.targetRot);
    cost += 0.5f * ik.alpha_poseDiff * (pose - origPose).squaredNorm();
    return cost;
}

VectorXf totalCostDerivative(const IKProblem& ik, glmx::transform endEffectorT,
        const MatrixXf& J,
        const VectorXf& pose, const VectorXf& origPose) {

    glm::vec3 dpos = endEffectorT.v - ik.targetPos;
    glm::vec3 drot = glmx::logdiff(endEffectorT.q, ik.targetRot);
    VectorXf df(6);
    df << ik.alpha_targetPos * glmx::toEigen(dpos), ik.alpha_targetRot * glmx::toEigen(drot);

    VectorXf dC = J.transpose() * df + ik.alpha_poseDiff * (pose - origPose);
    return dC;
}

VectorXf filterPoseVec(const VectorXf& poseVec, nonstd::span<uint32_t> relevantJoints) {
    VectorXf newPoseVec(relevantJoints.size() * 3);
    for (int c = 0; c < relevantJoints.size(); c++) {
        int i = relevantJoints[c];
        newPoseVec[3*c+0] = poseVec[3*i+0];
        newPoseVec[3*c+1] = poseVec[3*i+1];
        newPoseVec[3*c+2] = poseVec[3*i+2];
    }
    return newPoseVec;
}


VectorXf toEigen(const Pose& pose) {
    VectorXf poseVec(pose.size() * 3);
    for (int i = 0; i < pose.size(); i++) {
        glm::vec3 v = glmx::quatToEuler(pose.q[i], EulOrdXYZs);
        poseVec[3*i+0] = v[0];
        poseVec[3*i+1] = v[1];
        poseVec[3*i+2] = v[2];
    }
    return poseVec;
}

Pose toPose(glm::vec3 rootPos, const VectorXf& poseEuler) {
    int n = poseEuler.size()/3;
    Pose pose(rootPos, std::vector<glm::quat>(n));
    for (int i = 0; i < poseEuler.size()/3; i++) {
        pose.q[i] = glmx::eulerToQuat({poseEuler[3*i], poseEuler[3*i+1], poseEuler[3*i+2]}, EulOrdXYZs);
    }
    return pose;
}

void solveIK(const PoseTree& poseTree, Pose& pose, const IKProblem& ik) {
    VectorXf poseEuler = toEigen(pose);
    VectorXf origPoseEuler = poseEuler;

    float epsilon = 1e-4;
    float alpha0 = 100;
    float tau = 0.5;

    const int numIters = 1000;
    float cost, newCost;
    VectorXf newPoseEuler = poseEuler;

    // Gradient descent with Armijo line search
    int iter;
    for (iter = 0; iter < numIters; iter++) {
        std::vector<glmx::transform> Ts = calcFK(poseTree, pose.v, poseEuler);
        glmx::transform endEffectorT = Ts[ik.targetIdx];
        cost = totalCost(ik, endEffectorT, poseEuler, origPoseEuler);

        if (iter % 100 == 0) {
            printf("Iteration %d: cost = %f\n", iter, cost);
        }
        MatrixXf J = calcEulerJacobian(poseTree, Ts, ik.targetIdx, ik.relevantJoints);
        VectorXf dC = totalCostDerivative(ik, endEffectorT, J,
                filterPoseVec(poseEuler, ik.relevantJoints), filterPoseVec(origPoseEuler, ik.relevantJoints));

        float alpha = alpha0;
        for (int j = 0; j < 10; j++) {
            newPoseEuler = poseEuler;
            for (int k = 0; k < ik.relevantJoints.size(); k++) {
                int i = ik.relevantJoints[k];
                newPoseEuler[3*i+0] -= alpha * dC[3*k+0];
                newPoseEuler[3*i+1] -= alpha * dC[3*k+1];
                newPoseEuler[3*i+2] -= alpha * dC[3*k+2];
            }
            glmx::transform newEndEffectorT = calcFK(poseTree, pose.v, newPoseEuler)[ik.targetIdx];
            newCost = totalCost(ik, newEndEffectorT, newPoseEuler, origPoseEuler);

            float pred = 0.5f * dC.squaredNorm();
            if (cost - newCost >= alpha * pred) break;
            alpha *= tau;
        }
        if (iter % 100 == 0) {
            printf("alpha = %f\n", alpha);
        }

        float errorSq = (newPoseEuler - poseEuler).squaredNorm() / poseEuler.squaredNorm();
        poseEuler = newPoseEuler;
        if (errorSq < epsilon * epsilon) {
            break;
        }
    }
    pose = toPose(pose.v, poseEuler);

    printf("Optimization succeeded: iter=%d, cost=%f\n", iter, newCost);
}
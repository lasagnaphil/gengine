//
// Created by lasagnaphil on 19. 10. 1..
//

#include "PoseIK.h"
#include "PoseFK.h"
#include "glmx/pose.h"
#include <glmx/quat.h>
#include <glmx/eigen.h>

#include "Eigen/Dense"
#include "Eigen/Householder"

using namespace Eigen;

std::vector<glmx::transform> calcFK(const PoseTree& poseTree, glm::vec3 rootPos, const VectorXf& poseEuler) {
    std::vector<glmx::transform> transforms(poseTree.numNodes);
    std::stack<std::tuple<uint32_t, uint32_t>> recursionStack;

    transforms[0] = glmx::transform{rootPos, glm::identity<glm::quat>()};
    recursionStack.push({0, 0});

    while (!recursionStack.empty()) {
        auto[idx, parentIdx] = recursionStack.top();
        recursionStack.pop();

        auto& node = poseTree[idx];
        if (poseTree[idx].isEndSite()) {
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

glm::vec3 quatToTwist(glm::quat q) {
    float im_size = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z);
    glm::vec3 twist = glm::vec3(q.x, q.y, q.z);
    twist *= (2 * atan2f(im_size, q.w) / im_size);
    return twist;
}

MatrixXf calcEulerJacobian(const PoseTree& poseTree, nonstd::span<glmx::transform> Ts,
        uint32_t mIdx, nonstd::span<uint32_t> relevantJoints,
        nonstd::span<float> jointStiffness) {

    MatrixXf J(6, 3 * relevantJoints.size());

    int c = 0;
    for (uint32_t i : relevantJoints) {
        glmx::transform axisT = Ts[i];
        glm::vec3 v = Ts[mIdx].v - axisT.v;

        glm::vec3 wx = axisT.q * glm::vec3{1, 0, 0};
        J.block<3,1>(0, 3*c+0) = glmx::toEigen(glm::cross(wx, v)) / jointStiffness[c];
        J.block<3,1>(3, 3*c+0) = glmx::toEigen(wx) / jointStiffness[c];
        glm::vec3 wy = axisT.q * glm::vec3{0, 1, 0};
        J.block<3,1>(0, 3*c+1) = glmx::toEigen(glm::cross(wy, v)) / jointStiffness[c];
        J.block<3,1>(3, 3*c+1) = glmx::toEigen(wy) / jointStiffness[c];
        glm::vec3 wz = axisT.q * glm::vec3{0, 0, 1};
        J.block<3,1>(0, 3*c+2) = glmx::toEigen(glm::cross(wz, v)) / jointStiffness[c];
        J.block<3,1>(3, 3*c+2) = glmx::toEigen(wz) / jointStiffness[c];

        c++;
    }

    return J;
}

MatrixXf calcEulerJacobian(
        const PoseTree& poseTree, glmx::pose& pose,
        uint32_t mIdx, nonstd::span<uint32_t> relevantJoints,
        nonstd::span<float> jointStiffness) {

    std::vector<glmx::transform> Ts = calcFK(poseTree, pose);
    return calcEulerJacobian(poseTree, Ts, mIdx, relevantJoints, jointStiffness);
}



using Vector6f = Matrix<float, 6, -1>;


void
solveIKSimple(const PoseTree &poseTree, glmx::pose &pose, uint32_t mIdx,
              nonstd::span <uint32_t> relevantJoints,
              nonstd::span<float> jointStiffness,
              glmx::transform mT, LeastSquareMethod lsqMethod) {

    std::vector<glm::vec3> eulerAngles(pose.size());
    for (int i = 0; i < pose.size(); i++) {
        eulerAngles[i] = glmx::quatToEuler(pose.q[i], EulOrdXYZs);
    }
    MatrixXf J = calcEulerJacobian(poseTree, pose, mIdx, relevantJoints, jointStiffness);
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
        assert(i < eulerAngles.size());
        eulerAngles[i].x += dq[3*c+0] * jointStiffness[c];
        eulerAngles[i].y += dq[3*c+1] * jointStiffness[c];
        eulerAngles[i].z += dq[3*c+2] * jointStiffness[c];
        c++;
    }
    for (int i = 0; i < pose.size(); i++) {
        pose.q[i] = glmx::eulerToQuat(eulerAngles[i], EulOrdXYZs);
    }
}

void
solveIKSimple(const PoseTree &poseTree, glmx::pose &pose, uint32_t mIdx,
              nonstd::span <uint32_t> relevantJoints,
              nonstd::span<float> jointStiffness,
              glm::vec3 mPos, LeastSquareMethod lsqMethod) {

    std::vector<glm::vec3> eulerAngles(pose.size());
    for (int i = 0; i < pose.size(); i++) {
        eulerAngles[i] = glmx::quatToEuler(pose.q[i], EulOrdXYZs);
    }
    MatrixXf J = calcEulerJacobian(poseTree, pose, mIdx, relevantJoints, jointStiffness)
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
        assert(i < eulerAngles.size());
        eulerAngles[i].x += dq[3*c+0] * jointStiffness[c];
        eulerAngles[i].y += dq[3*c+1] * jointStiffness[c];
        eulerAngles[i].z += dq[3*c+2] * jointStiffness[c];
        c++;
    }

    for (int i = 0; i < pose.size(); i++) {
        pose.q[i] = glmx::eulerToQuat(eulerAngles[i], EulOrdXYZs);
    }
}

void
solveTwoJointIK(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 target,
        glm::quat a_gr, glm::quat b_gr, glm::quat& a_lr, glm::quat& b_lr, float eps) {

    using namespace glm;

    float lab = length(b - a);
    float lcb = length(b - c);
    float lat = clamp(length(target - a), eps, lab + lcb - eps);


    float ac_ab_0 = acos(clamp(dot(normalize(c - a), normalize(b - a)), -1.f, 1.f));
    float ba_bc_0 = acos(clamp(dot(normalize(a - b), normalize(c - b)), -1.f, 1.f));
    float ac_at_0 = acos(clamp(dot(normalize(c - a), normalize(target - a)), -1.f, 1.f));

    float ac_ab_1 = acos(clamp((lcb*lcb-lab*lab-lat*lat) / (-2*lab*lat), -1.f, 1.f));
    float ba_bc_1 = acos(clamp((lat*lat-lab*lab-lcb*lcb) / (-2*lab*lcb), -1.f, 1.f));

    vec3 axis0 = normalize(cross(c - a, b - a));
    vec3 axis1 = normalize(cross(c - a, target - a));

    quat r0 = angleAxis(ac_ab_1 - ac_ab_0, inverse(a_gr) * axis0);
    quat r1 = angleAxis(ba_bc_1 - ba_bc_0, inverse(b_gr) * axis0);
    quat r2 = angleAxis(ac_at_0, inverse(a_gr) * axis1);

    a_lr = a_lr * r0 * r2;
    b_lr = b_lr * r1;
}

void
solveTwoJointIK(const PoseTree &poseTree, glmx::pose &pose, uint32_t aIdx, uint32_t bIdx, uint32_t cIdx,
                glm::vec3 target, float eps) {
    glmx::transform aT = calcFK(poseTree, pose, aIdx);
    glmx::transform bT = calcFK(poseTree, pose, bIdx);
    glmx::transform cT = calcFK(poseTree, pose, cIdx);

    solveTwoJointIK(aT.v, bT.v, cT.v, target, aT.q, bT.q, pose.q[aIdx], pose.q[bIdx], eps);
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
    int c;
    for (c = 0; c < relevantJoints.size(); c++) {
        int i = relevantJoints[c];
        newPoseVec[3*c+0] = poseVec[3*i+0];
        newPoseVec[3*c+1] = poseVec[3*i+1];
        newPoseVec[3*c+2] = poseVec[3*i+2];
    }
    return newPoseVec;
}


VectorXf toEigen(const glmx::pose& pose) {
    VectorXf poseVec(pose.size() * 3);
    int i;
    for (i = 0; i < pose.size(); i++) {
        glm::vec3 v = glmx::quatToEuler(pose.q[i], EulOrdXYZs);
        poseVec[3*i+0] = v[0];
        poseVec[3*i+1] = v[1];
        poseVec[3*i+2] = v[2];
    }
    return poseVec;
}

glmx::pose toPose(glm::vec3 rootPos, const VectorXf& poseEuler) {
    int n = poseEuler.size()/3;
    glmx::pose pose(rootPos, std::vector<glm::quat>(n));
    for (int i = 0; i < n; i++) {
        pose.q[i] = glmx::eulerToQuat({poseEuler[3*i], poseEuler[3*i+1], poseEuler[3*i+2]}, EulOrdXYZs);
    }
    return pose;
}

void solveIK(const PoseTree& poseTree, glmx::pose& pose, const IKProblem& ik) {
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
        MatrixXf J = calcEulerJacobian(poseTree, Ts, ik.targetIdx, ik.relevantJoints, ik.jointStiffness);
        VectorXf dC = totalCostDerivative(ik, endEffectorT, J,
                filterPoseVec(poseEuler, ik.relevantJoints),
                filterPoseVec(origPoseEuler, ik.relevantJoints));

        float alpha = alpha0;
        for (int j = 0; j < 10; j++) {
            newPoseEuler = poseEuler;
            int n = ik.relevantJoints.size();
            for (int k = 0; k < n; k++) {
                int i = ik.relevantJoints[k];
                newPoseEuler[3*i+0] -= alpha * dC[3*k+0] * ik.jointStiffness[k];
                newPoseEuler[3*i+1] -= alpha * dC[3*k+1] * ik.jointStiffness[k];
                newPoseEuler[3*i+2] -= alpha * dC[3*k+2] * ik.jointStiffness[k];
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


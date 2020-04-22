//
// Created by lasagnaphil on 19. 6. 19.
//

#ifndef PHYSICS_BENCHMARKS_POSE_H
#define PHYSICS_BENCHMARKS_POSE_H

#include <glmx/quat.h>
#include <glmx/transform.h>

#include <vector>
#include <glm/vec3.hpp>

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include <glm/gtx/euler_angles.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glmx/euler.h>

#include "anim/PoseTree.h"

/*
 * Represents a pose of an aritculated figure.
 * In mathematical terms, P = (v0, q1, q2, ..., qn).
 */

namespace glmx {
    struct pose_disp;

    struct pose_view {
        float* v_ptr = nullptr;
        float* q_ptr = nullptr;
        size_t size = 0;

        pose_view() = default;
        pose_view(float* v_ptr, float* q_ptr, size_t size) :
            v_ptr(v_ptr), q_ptr(q_ptr), size(size) {}
        pose_view(glm::vec3* v_ptr, glm::quat* q_ptr, size_t size) :
                v_ptr((float*)v_ptr), q_ptr((float*)q_ptr), size(size) {}

        glm::vec3& v() {
            return *((glm::vec3*)v_ptr);
        }

        const glm::vec3& v() const {
            return *((glm::vec3*)v_ptr);
        }

        glm::quat& q(size_t i) {
            assert(i < size);
            return ((glm::quat*)q_ptr)[i];
        }

        const glm::quat& q(size_t i) const {
            assert(i < size);
            return ((glm::quat*)q_ptr)[i];
        }

        glmx::transform getRoot() {
            return {v(), q(0)};
        }
    };

    struct pose {
        glm::vec3 v;
        std::vector<glm::quat> q;

        pose() = default;

        pose(glm::vec3 rootPos, std::vector<glm::quat> jointRot) :
                v(rootPos), q(std::move(jointRot)) {}
        pose(pose_view view) {
            v = view.v();
            glm::quat* q_ptr = (glm::quat*)view.q_ptr;
            q = std::vector<glm::quat>(q_ptr, q_ptr + view.size);
        }

        static pose empty(std::size_t n) {
            auto q = std::vector<glm::quat>(n, glm::identity<glm::quat>());
            return {glm::vec3(0.0f), q};
        }

        size_t size() const { return q.size(); }

        pose_view getView() {
            return pose_view(&v, q.data(), q.size());
        }

        glmx::transform getRoot() {
            return {v, q[0]};
        }
    };

    inline void slerp(pose_view p1, pose_view p2, float alpha, pose_view p3) {
        assert(p1.size == p2.size && p1.size == p3.size);
        p3.v() = (1 - alpha) * p1.v() + alpha * p2.v();
        for (size_t i = 0; i < p3.size; i++) {
            p3.q(i) = glm::slerp(p1.q(i), p2.q(i), alpha);
        }
    }

    inline void lerp(pose_view p1, pose_view p2, float alpha, pose_view p3) {
        assert(p1.size == p2.size && p1.size == p3.size);
        p3.v() = (1 - alpha) * p1.v() + alpha * p2.v();
        for (size_t i = 0; i < p3.size; i++) {
            p3.q(i) = glm::normalize(glm::lerp(p1.q(i), p2.q(i), alpha));
        }
    }

    inline void slerp(const pose& p1, const pose& p2, float alpha, pose& p3) {
        p3.q.resize(p1.size());
        assert(p1.size() == p2.size());
        p3.v = (1 - alpha) * p1.v + alpha * p2.v;
        for (size_t i = 0; i < p3.size(); i++) {
            p3.q[i] = glm::slerp(p1.q[i], p2.q[i], alpha);
        }
    }

    inline void lerp(const pose& p1, const pose& p2, float alpha, pose& p3) {
        p3.q.resize(p1.size());
        assert(p1.size() == p2.size());
        p3.v = (1 - alpha) * p1.v + alpha * p2.v;
        for (size_t i = 0; i < p3.size(); i++) {
            p3.q[i] = glm::normalize(glm::lerp(p1.q[i], p2.q[i], alpha));
        }
    }
}

#endif //PHYSICS_BENCHMARKS_POSE_H

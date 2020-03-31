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

#include "PoseTree.h"

/*
 * The mathmatical formulation of the following classes are at:
 * http://mrl.snu.ac.kr/publications/RotationAndOrientation.pdf
 */


/*
 * Represents a pose of an aritculated figure.
 * In mathematical terms, P = (v0, q1, q2, ..., qn).
 */

namespace glmx {
    struct pose_disp;

    struct pose {
        glm::vec3 v;
        std::vector<glm::quat> q;

        pose() = default;

        pose(glm::vec3 rootPos, std::vector<glm::quat> jointRot) :
                v(rootPos), q(std::move(jointRot)) {}

        static pose empty(std::size_t n) {
            auto q = std::vector<glm::quat>(n, glm::identity<glm::quat>());
            return {glm::vec3(0.0f), q};
        }

        std::size_t size() const { return q.size(); }

        glmx::transform getRoot() {
            return {v, q[0]};
        }
    };

    struct pose_euler {
        glm::vec3 v;
        std::vector<glm::vec3> eulerAngles;

        pose_euler() = default;

        pose_euler(glm::vec3 rootPos, std::vector<glm::vec3> jointRot) :
                v(rootPos), eulerAngles(std::move(jointRot)) {}

        static pose_euler empty(std::size_t n) {
            auto q = std::vector<glm::vec3>(n, glm::vec3(0.0f, 0.0f, 0.0f));
            return {glm::vec3(0.0f), q};
        }

        std::size_t size() const { return eulerAngles.size(); }
    };

    inline pose_euler toEuler(const pose& p, int order) {
        pose_euler pe = pose_euler::empty(p.size());
        pe.v = p.v;
        for (size_t i = 0; i < pe.size(); i++) {
            pe.eulerAngles[i] = glmx::quatToEuler(p.q[i], order);
        }
        return pe;
    }

    inline pose toQuat(const pose_euler& pe) {
        pose p = pose::empty(pe.size());
        p.v = pe.v;
        for (size_t i = 0; i < p.size(); i++) {
            glm::vec3 e = pe.eulerAngles[i];
            p.q[i] = glmx::eulerToQuat(e);
        }
        return p;
    }

    inline pose slerp(const pose& p1, const pose& p2, float alpha) {
        assert(p1.size() == p2.size());
        pose p = pose::empty(p1.size());

        p.v = (1 - alpha) * p1.v + alpha * p2.v;
        for (size_t i = 0; i < p.size(); i++) {
            p.q[i] = glm::slerp(p1.q[i], p2.q[i], alpha);
        }

        return p;
    }

    /*
     * Represents the displacement of a pose.
     * In mathematical terms, D = (d0, d1, ..., dn).
     */
    struct pose_disp {
        std::vector<glm::vec3> diff;

        std::size_t size() const { return diff.size() - 1; }

        pose_disp() = default;

        explicit pose_disp(std::vector<glm::vec3> diff) : diff(std::move(diff)) {}

        static pose_disp empty(std::size_t n) {
            return pose_disp(std::vector<glm::vec3>(n + 1, glm::vec3(0.0f)));
        }

        const glm::vec3& operator[](std::size_t i) const { return diff[i]; }

        glm::vec3& operator[](std::size_t i) { return diff[i]; }
    };

    inline void add(pose& pRes, const pose& p, const pose_disp& d) {
        assert(pRes.size() == p.size());
        assert(p.size() == d.size());

        pRes.v = p.q[0] * p.v + d[0];
        for (size_t i = 0; i < pRes.size(); i++) {
            pRes.q[i] = p.q[i] * glmx::exp(d[i + 1]);
        }
    }

    inline void add(pose& p, const pose_disp& d) {
        add(p, p, d);
    }

    inline void disp(pose_disp& d, const pose& p1, const pose& p2) {
        assert(d.size() == p1.size());
        assert(p1.size() == p2.size());

        d.diff[0] = glm::conjugate(p2.q[0]) * (p1.v - p2.v);
        for (size_t i = 0; i < d.size(); i++) {
            d.diff[i+1] = glmx::log(glm::conjugate(p2.q[i]) * p1.q[i]);
        }
    }

    inline void mult(pose_disp& pRes, const pose_disp& p, float alpha) {
        assert(pRes.size() == p.size());
        for (size_t i = 0; i < p.size() + 1; i++) {
            pRes.diff[i] = alpha * p.diff[i];
        }
    }

    inline void mult(pose_disp& d, float alpha) {
        mult(d, d, alpha);
    }

    inline void translate_root(pose& p, glm::vec3 v) {
        p.v += p.q[0] * v;
    }

    inline void rotate_root(pose& p, glm::quat q) {
        p.q[0] = p.q[0] * q;
    }

    inline void transform_root(pose& p, const glmx::transform& t) {
        p.v += p.q[0] * t.v;
        p.q[0] = p.q[0] * t.q;
    }

    inline pose& operator+=(pose& p, const pose_disp& d) {
        add(p, p, d);
        return p;
    }

    inline pose_disp& operator*=(pose_disp& d, float alpha) {
        mult(d, d, alpha);
        return d;
    }

    void renderPoseImGui(pose& pose, const PoseTree& poseTree);
}

#endif //PHYSICS_BENCHMARKS_POSE_H

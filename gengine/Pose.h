//
// Created by lasagnaphil on 19. 6. 19.
//

#ifndef PHYSICS_BENCHMARKS_POSE_H
#define PHYSICS_BENCHMARKS_POSE_H

#include <vector>
#include <glm/vec3.hpp>

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>

/*
 * The mathmatical formulation of the following classes are at:
 * http://mrl.snu.ac.kr/publications/RotationAndOrientation.pdf
 */

struct PoseDisp;

/*
 * Represents a pose of an aritculated figure.
 * In mathematical terms, P = (v0, q1, q2, ..., qn).
 * Can be in global or local coordinates. Use toGlobal() and toLocal() to switch between them.
 * Note that mathematical functions (pose arithmetic) only works on local coordinates!
 */
struct Pose {
    glm::vec3 v;
    std::vector<glm::quat> q;

    Pose() = default;
    Pose(glm::vec3 rootPos, std::vector<glm::quat> jointRot) :
        v(rootPos), q(std::move(jointRot)) {}

    static Pose empty(std::size_t n) {
        auto q = std::vector<glm::quat>(n, glm::identity<glm::quat>());
        return {glm::vec3 {0.f, 0.f, 0.f}, q};
    }

    std::size_t size() const { return q.size(); }
};

/*
 * Represents the displacement of a pose.
 * In mathematical terms, D = (d0, d1, ..., dn).
 */
struct PoseDisp {
    std::vector<glm::vec3> diff;

    std::size_t size() const { return diff.size() - 1; }

    PoseDisp() = default;
    explicit PoseDisp(std::vector<glm::vec3> diff) : diff(std::move(diff)) {}

    static PoseDisp empty(std::size_t n) {
        return PoseDisp(std::vector<glm::vec3>(n + 1, glm::vec3(0.0f)));
    }

    const glm::vec3& operator[](std::size_t i) const { return diff[i]; }
    glm::vec3& operator[](std::size_t i) { return diff[i]; }
};

inline Pose slerp(const Pose& p1, const Pose& p2, float alpha) {
    assert(p1.size() == p2.size());
    Pose p = Pose::empty(p1.size());

    p.v = alpha * p1.v + (1 - alpha) * p2.v;
    for (int i = 0; i < p.size(); i++) {
        p.q[i] = glm::slerp(p1.q[i], p2.q[i], alpha);
    }

    return p;
}

inline Pose operator*(const Pose& p1, const Pose& p2) {
    assert(p1.size() == p2.size());

    Pose p;
    p.v = p1.q[0] * p2.v + p1.v;
    p.q.resize(p1.size());
    for (int i = 0; i < p.q.size(); i++) {
        p.q[i] = p1.q[i] * p2.q[i];
    }
    return p;
}

inline Pose operator/(const Pose& p1, const Pose& p2) {
    assert(p1.size() == p2.size());

    Pose p = Pose::empty(p1.size());
    p.v = glm::conjugate(p2.q[0]) * (p1.v - p2.v);
    for (int i = 0; i < p.q.size(); i++) {
        p.q[i] = glm::conjugate(p2.q[i]) * p1.q[i];
    }
    return p;
}

inline PoseDisp operator+(const PoseDisp& d1, const PoseDisp& d2) {
    assert(d1.size() == d2.size());

    PoseDisp d = PoseDisp::empty(d1.size());
    d.diff.resize(d1.size());
    for (int i = 0; i < d.size(); i++) {
        d[i] = d1[i] + d2[i];
    }
    return d;
}

inline PoseDisp operator-(const PoseDisp& d1, const PoseDisp& d2) {
    assert(d1.size() == d2.size());

    PoseDisp d = PoseDisp::empty(d1.size());
    for (int i = 0; i < d.size(); i++) {
        d[i] = d1[i] - d2[i];
    }
    return d;
}

inline PoseDisp operator*(float k, const PoseDisp& d) {
    PoseDisp dnew = PoseDisp::empty(d.size());
    for (int i = 0; i < d.size(); i++) {
        dnew[i] = k * d[i];
    }
    return dnew;
}

inline PoseDisp operator*(const PoseDisp& d, float k) {
    return k * d;
}

inline Pose exp(const PoseDisp& d) {
    Pose p = Pose::empty(d.size());
    p.v = d[0];
    for (int i = 0; i < d.size() - 1; i++) {
        glm::vec3 u = d[i + 1] / 2.0f;
        p.q[i] = glm::angleAxis(glm::length(u), glm::normalize(u));
    }
    return p;
}

inline PoseDisp log(const Pose& p) {
    PoseDisp d = PoseDisp::empty(p.size());
    d[0] = p.v;
    for (int i = 0; i < p.size(); i++) {
        d[i + 1] = glm::angle(p.q[i]) * glm::axis(p.q[i]);
    }
    return d;
}

void renderImGui(const Pose& pose);

#endif //PHYSICS_BENCHMARKS_POSE_H

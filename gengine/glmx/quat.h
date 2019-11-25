//
// Created by lasagnaphil on 19. 11. 7..
//

#ifndef GENGINE_QUAT_H
#define GENGINE_QUAT_H

#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include "euler.h"

namespace glmx {
    inline glm::vec3 log(glm::quat q) {
        q = glm::normalize(q);
        float a = sqrtf(1 - q.w*q.w);
        if (a <= glm::epsilon<float>()) {
            return glm::vec3 {};
        }
        return 2.0f * atan2f(a, q.w) / a * glm::vec3(q.x, q.y, q.z);
    }

    inline glm::vec3 logdiff(glm::quat q1, glm::quat q2) {
        return glmx::log(q2 * glm::conjugate(q1));
    }

    inline glm::quat exp(glm::vec3 v) {
        float s = glm::length(v);
        if (s <= glm::epsilon<float>()) {
            return glm::identity<glm::quat>();
        }
        glm::vec3 u = v / s;
        return glm::quat(s, u);
    }

    inline float extractYRot(glm::quat q) {
        if (q.y * q.y + q.w * q.w <= glm::epsilon<float>()) {
            return 0.0f;
        }
        return 2 * glm::atan(q.y, q.w);
    }
}

#endif //GENGINE_QUAT_H

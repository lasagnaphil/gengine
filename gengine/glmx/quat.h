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
        float theta = glm::length(v);
        if (theta <= glm::epsilon<float>()) {
            return glm::identity<glm::quat>();
        }
        glm::vec3 u = v / theta;
        return glm::quat(glm::cos(theta/2), glm::sin(theta/2) * u);
    }

    inline float extractYRot(glm::quat q) {
        if (q.y * q.y + q.w * q.w <= glm::epsilon<float>()) {
            return 0.0f;
        }
        return 2 * glm::atan(q.y, q.w);
    }

    inline glm::mat4 rotMatrixBetweenVecs(glm::vec3 a, glm::vec3 b) {
        glm::vec3 v = glm::cross(a, b);
        float s2 = glm::dot(v, v);
        if (s2 < glm::epsilon<float>()) {
            return glm::mat4(1.0f);
        }
        else {
            // Rodrigue's formula
            float c = glm::dot(a, b);
            glm::mat3 vhat;
            vhat[0][0] = vhat[1][1] = vhat[2][2] = 0;
            vhat[2][1] = v[0]; vhat[1][2] = -v[0];
            vhat[0][2] = v[1]; vhat[2][0] = -v[1];
            vhat[1][0] = v[2]; vhat[0][1] = -v[2];
            return glm::mat3(1.0f) + vhat + vhat*vhat*(1 - c)/(s2);
        }
    }

    inline glm::quat quatBetweenVecs(glm::vec3 a, glm::vec3 b) {
        glm::vec3 w = glm::cross(a, b);
        glm::quat q = glm::quat(1.f + dot(a, b), w);
        return glm::normalize(q);
    }


}

#endif //GENGINE_QUAT_H

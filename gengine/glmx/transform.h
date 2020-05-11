//
// Created by lasagnaphil on 19. 10. 1..
//

#ifndef GENGINE_GLMX_TRANSFORM_H
#define GENGINE_GLMX_TRANSFORM_H

#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace glmx {
    struct transform {
        glm::vec3 v;
        glm::quat q;

        transform() : v(0.0f), q(glm::identity<glm::quat>()) {}
        transform(glm::vec3 v) : v(v), q(glm::identity<glm::quat>()) {}
        transform(glm::quat q) : v(glm::vec3()), q(q) {}
        transform(glm::vec3 v, glm::quat q) : v(v), q(q) {}
    };

    struct transform_disp {
        glm::vec3 v;
        glm::vec3 u;

        transform_disp() = default;
        transform_disp(glm::vec3 v, glm::vec3 u) : v(v), u(u) {}
    };

    inline glm::mat4 mat4_cast(const transform& t) {
        return glm::translate(t.v) * mat4_cast(t.q);
    }

    inline transform operator*(const transform& t1, const transform& t2) {
        return {t1.q * t2.v + t1.v, t1.q * t2.q};
    }

    inline transform operator/(const transform& t1, const transform& t2) {
        return {conjugate(t2.q) * (t1.v - t2.v), glm::conjugate(t2.q) * t1.q};
    }

    inline transform_disp operator+(const transform_disp& d1, const transform_disp& d2) {
        return {d1.v + d2.v, d1.u + d2.u};
    }

    inline transform_disp operator-(const transform_disp& d1, const transform_disp& d2) {
        return {d1.v - d2.v, d1.u - d2.u};
    }

    inline transform_disp operator*(float k, const transform_disp& d) {
        return {k * d.v, k * d.u};
    }

    inline transform_disp operator/(float k, const transform_disp& d) {
        return {d.v / k, d.u / k};
    }

    inline transform exp(const transform_disp& d) {
        return {d.v, glm::angleAxis(glm::length(d.u), glm::normalize(d.u))};
    }

    inline transform_disp log(const transform& t) {
        return {t.v, glm::angle(t.q) * glm::axis(t.q)};
    }

}

#endif //GENGINE_TRANSFORM_H

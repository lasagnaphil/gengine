//
// Created by lasagnaphil on 20. 1. 6..
//

#ifndef LATENT_MOTION_INTERP_H
#define LATENT_MOTION_INTERP_H

namespace glmx {

inline std::pair<glm::quat, glm::quat> squad_s(
        glm::quat q0, glm::quat q1, glm::quat q2, glm::quat q3, float t) {

    glm::quat s1 = q1 * glmx::exp(-0.25f * (glmx::logdiff(q1, q2) + glmx::logdiff(q1, q0)));
    glm::quat s2 = q2 * glmx::exp(-0.25f * (glmx::logdiff(q2, q3) + glmx::logdiff(q2, q1)));
    return {s1, s2};
}

inline glm::quat squad_q(glm::quat q1, glm::quat q2, glm::quat s1, glm::quat s2, float t) {
    return glm::slerp(glm::slerp(q1, q2, t), glm::slerp(s1, s2, t), 2*t*(1-t));
}

inline glm::quat squad(glm::quat q0, glm::quat q1, glm::quat q2, glm::quat q3, float t) {
    auto [s1, s2] = squad_s(q0, q1, q2, q3, t);
    return squad_q(q1, q2, s1, s2, t);
}

}
#endif //LATENT_MOTION_INTERP_H

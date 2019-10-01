//
// Created by lasagnaphil on 19. 10. 1..
//

#ifndef GENGINE_GLMX_EIGEN_H
#define GENGINE_GLMX_EIGEN_H

#include "Eigen/Dense"

inline Eigen::Vector3f GLMToEigen(const glm::vec3& v) {
    return Eigen::Vector3f(v.x, v.y, v.z);
}

#endif //GENGINE_EIGEN_H

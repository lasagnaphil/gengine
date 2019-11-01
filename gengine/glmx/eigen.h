//
// Created by lasagnaphil on 19. 10. 1..
//

#ifndef GENGINE_GLMX_EIGEN_H
#define GENGINE_GLMX_EIGEN_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "eigen_raw.h"
#include "Eigen/Dense"

namespace glmx {
    inline Eigen::Map<Eigen::VectorXf> toEigen(RawVectorXf v) {
        return Eigen::Map<Eigen::VectorXf>(v.dataPtr, v.size);
    }
    inline Eigen::Map<Eigen::VectorXd> toEigen(RawVectorXd v) {
        return Eigen::Map<Eigen::VectorXd>(v.dataPtr, v.size);
    }
    inline Eigen::Map<Eigen::MatrixXf> toEigen(RawMatrixXf m) {
        return Eigen::Map<Eigen::MatrixXf>(m.dataPtr, m.rows, m.cols);
    }
    inline Eigen::Map<Eigen::MatrixXd> toEigen(RawMatrixXd m) {
        return Eigen::Map<Eigen::MatrixXd>(m.dataPtr, m.rows, m.cols);
    }
    inline RawVectorXf toRaw(Eigen::VectorXf& v) {
        return {v.data(), (uint32_t) v.size()};
    }
    inline RawVectorXd toRaw(Eigen::VectorXd& v) {
        return {v.data(), (uint32_t) v.size()};
    }
    inline RawVectorXf toRaw(Eigen::Map<Eigen::VectorXf>& v) {
        return {v.data(), (uint32_t) v.size()};
    }
    inline RawVectorXd toRaw(Eigen::Map<Eigen::VectorXd>& v) {
        return {v.data(), (uint32_t) v.size()};
    }
    inline RawMatrixXf toRaw(Eigen::MatrixXf& m) {
        return {m.data(), (uint32_t) m.rows(), (uint32_t) m.cols()};
    }
    inline RawMatrixXd toRaw(Eigen::MatrixXd& m) {
        return {m.data(), (uint32_t) m.rows(), (uint32_t) m.cols()};
    }
    inline RawMatrixXf toRaw(Eigen::Map<Eigen::MatrixXf>& m) {
        return {m.data(), (uint32_t) m.rows(), (uint32_t) m.cols()};
    }
    inline RawMatrixXd toRaw(Eigen::Map<Eigen::MatrixXd>& m) {
        return {m.data(), (uint32_t) m.rows(), (uint32_t) m.cols()};
    }

    inline Eigen::Vector2f toEigen(glm::vec2 v) {
        return Eigen::Vector2f(v.x, v.y);
    }
    inline Eigen::Vector3f toEigen(glm::vec3 v) {
        return Eigen::Vector3f(v.x, v.y, v.z);
    }
    inline Eigen::Vector4f toEigen(glm::vec4 v) {
        return Eigen::Vector4f(v.x, v.y, v.z, v.w);
    }
}

#endif //GENGINE_EIGEN_H

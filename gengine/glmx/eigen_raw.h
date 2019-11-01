//
// Created by lasagnaphil on 19. 11. 1..
//

#ifndef GENGINE_EIGEN_COMPAT_H
#define GENGINE_EIGEN_COMPAT_H

#include <cstdint>

struct RawVectorXf {
    float* dataPtr;
    uint32_t size;
};

struct RawVectorXd {
    double* dataPtr;
    uint32_t size;
};

struct RawMatrixXf {
    float* dataPtr;
    uint32_t rows;
    uint32_t cols;
};

struct RawMatrixXd {
    double* dataPtr;
    uint32_t rows;
    uint32_t cols;
};


#endif //GENGINE_EIGEN_COMPAT_H

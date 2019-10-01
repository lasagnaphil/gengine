//
// Created by lasagnaphil on 19. 10. 1..
//

#ifndef GENGINE_INVERSEKINEMATICS_H
#define GENGINE_INVERSEKINEMATICS_H

#include <glmx/transform.h>
#include <glmx/eigen.h>
#include <glmx/euler.h>

#include "Pose.h"
#include "MotionClipData.h"

glmx::transform calcFK(const PoseTree& poseTree, const Pose& pose, uint32_t mIdx);

void solveIK(const PoseTree& poseTree, Pose& pose, uint32_t mIdx, nonstd::span<uint32_t> relevantJoints, glm::vec3 mPos);

#endif //GENGINE_INVERSEKINEMATICS_H

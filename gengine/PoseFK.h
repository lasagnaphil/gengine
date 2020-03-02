//
// Created by lasagnaphil on 20. 2. 12..
//

#ifndef DEEPMIMIC_POSEFK_H
#define DEEPMIMIC_POSEFK_H

#include "glmx/pose.h"
#include "glmx/transform.h"
#include "PoseTree.h"

glmx::transform calcFK(const PoseTree& poseTree, const glmx::pose& pose, uint32_t mIdx);

std::vector<glmx::transform> calcFK(const PoseTree& poseTree, const glmx::pose& pose);

#endif //DEEPMIMIC_POSEFK_H
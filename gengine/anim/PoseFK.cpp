//
// Created by lasagnaphil on 20. 2. 12..
//

#include "PoseFK.h"

glmx::transform calcFK(const PoseTree &poseTree, glmx::const_pose_view pose, uint32_t mIdx) {
    uint32_t i = mIdx;
    if (poseTree[i].isEndSite()) {
        i = poseTree[i].parent;
    }

    glmx::transform t(glm::vec3(0.0f), glm::identity<glm::quat>());

    while (true) {
        auto& node = poseTree[i];
        if (poseTree[i].isEndSite()) {
            t = glmx::transform(node.offset) * t;
        }
        else {
            t = glmx::transform(node.offset, pose.q(i)) * t;
        }
        if (i == 0) {
            t = glmx::transform(pose.v()) * t;
            break;
        }
        else {
            i = node.parent;
        }
    }
    return t;
}

std::vector<glmx::transform> calcFK(const PoseTree& poseTree, glmx::const_pose_view pose) {
    std::vector<glmx::transform> transforms(poseTree.numNodes);
    std::stack<std::tuple<uint32_t, uint32_t>> recursionStack;

    transforms[0] = glmx::transform{pose.v(), glm::identity<glm::quat>()};
    recursionStack.push({0, 0});

    while (!recursionStack.empty()) {
        auto[idx, parentIdx] = recursionStack.top();
        recursionStack.pop();

        auto& node = poseTree[idx];
        if (poseTree[idx].isEndSite()) {
            transforms[idx] = transforms[parentIdx] * glmx::transform(node.offset);
        } else {
            transforms[idx] = transforms[parentIdx] * glmx::transform(node.offset, pose.q(idx));
        }

        for (uint32_t childIdx : node.childJoints) {
            recursionStack.push({childIdx, idx});
        }
    }

    return transforms;
}



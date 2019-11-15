//
// Created by lasagnaphil on 19. 6. 19.
//

#include "pose.h"
#include "MotionClipData.h"
#include "Utils.h"
#include <imgui.h>

void renderPoseImGui(const glmx::pose& pose, const PoseTree& poseTree) {
    std::stack<uint32_t> recursionStack;
    recursionStack.push(0);

    while (!recursionStack.empty()) {
        uint32_t nodeIdx = recursionStack.top();
        recursionStack.pop();

        const PoseTreeNode& node = poseTree.allNodes[nodeIdx];
        if (ImGui::TreeNode("%s##%s", node.name.c_str(), node.name.c_str())) {
            for (int i = node.childJoints.size(); i >= 0; i--) {
                recursionStack.push(node.childJoints[i]);
            }
            ImGui::TreePop();
        }
    }

}


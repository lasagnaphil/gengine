//
// Created by lasagnaphil on 19. 9. 23..
//

#ifndef GENGINE_POSERENDERBODY_H
#define GENGINE_POSERENDERBODY_H

#include "Pose.h"
#include "MotionClipData.h"

#include <glm/gtx/transform.hpp>
#include <deps/glm/glm/gtx/string_cast.hpp>

inline glm::mat4 rotationBetweenVecs(glm::vec3 a, glm::vec3 b) {
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

struct PoseRenderBody {
    std::vector<Ref<Mesh>> meshes;
    std::vector<Ref<Material>> materials;

    static PoseRenderBody createAsBoxes(const PoseTree& poseTree, float width, Ref<Material> material) {
        PoseRenderBody body;
        body.meshes.resize(poseTree.numNodes - 1);
        body.materials.resize(poseTree.numNodes - 1);

        for (int i = 1; i < poseTree.numNodes; i++) {
            float offsetLength = glm::length(poseTree[i].offset);
            if (offsetLength > 0) {
                body.meshes[i-1] = Mesh::makeCube({width, offsetLength, width});
            }
            else {
                body.meshes[i-1] = {};
            }
            body.materials[i-1] = material;
        }

        return body;
    }
};

inline void renderMotionClip(PhongRenderer& renderer,
        const Pose& poseState, const PoseTree& poseTree, const PoseRenderBody& body,
        const glm::mat4& globalTrans = glm::mat4(1.0f)) {

    // Recursively render all nodes
    std::stack<std::tuple<uint32_t, glm::mat4>> recursionStack;
    recursionStack.push({0, glm::translate(globalTrans, poseState.v)});
    while (!recursionStack.empty()) {
        auto [nodeIdx, curTransform] = recursionStack.top();
        recursionStack.pop();

        const PoseTreeNode& node = poseTree[nodeIdx];
        if (!node.isEndSite) {
            // render bone related to current node (we exclude root node)
            if (nodeIdx != 0 && glm::length(node.offset) > glm::epsilon<float>()) {
                glm::vec3 a = glm::normalize(node.offset);
                glm::vec3 b = {0, 1, 0};
                glm::mat4 initialRot, initialTrans;
                if (node.offset.y >= 0) {
                    initialRot = rotationBetweenVecs(a, b);
                    initialTrans = glm::translate(glm::vec3{0.0f, glm::length(node.offset)/2, 0.0f});
                }
                else {
                    initialRot = rotationBetweenVecs(a, -b);
                    initialTrans = glm::translate(glm::vec3{0.0f, -glm::length(node.offset)/2, 0.0f});
                }
                glm::mat4 initialBoneTransform = initialRot * initialTrans;

                if (body.meshes[nodeIdx - 1]) {
                    renderer.queueRender(RenderCommand {
                            body.meshes[nodeIdx - 1],
                            body.materials[nodeIdx - 1],
                            curTransform * initialBoneTransform
                    });
                }
            }

            curTransform = curTransform * glm::translate(node.offset) * glm::mat4_cast(poseState.q[nodeIdx]);
            for (auto childID : node.childJoints) {
                recursionStack.push({childID, curTransform});
            }
        }
    }
}

#endif //GENGINE_POSERENDERBODY_H

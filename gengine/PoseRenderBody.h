//
// Created by lasagnaphil on 19. 9. 23..
//

#ifndef GENGINE_POSERENDERBODY_H
#define GENGINE_POSERENDERBODY_H

#include "glmx/pose.h"
#include "glmx/quat.h"
#include "Mesh.h"
#include "MotionClipData.h"
#include "PhongRenderer.h"
#include "DebugRenderer.h"

#include <glm/gtx/transform.hpp>
#include <deps/glm/glm/gtx/string_cast.hpp>


struct PoseRenderBody {
    std::vector<Ref<Mesh>> meshes;
    std::vector<Ref<PhongMaterial>> materials;

    static PoseRenderBody createAsBoxes(const PoseTree& poseTree, float width, Ref<PhongMaterial> material) {
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

inline void renderMotionClip(PhongRenderer& renderer, DebugRenderer& imRenderer,
                             const glmx::pose& poseState, const PoseTree& poseTree, const PoseRenderBody& body,
                             const glm::mat4& globalTrans = glm::mat4(1.0f), bool debug = false) {

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
                    initialRot = glmx::rotMatrixBetweenVecs(a, b);
                    initialTrans = glm::translate(glm::vec3{0.0f, glm::length(node.offset)/2, 0.0f});
                }
                else {
                    initialRot = glmx::rotMatrixBetweenVecs(a, -b);
                    initialTrans = glm::translate(glm::vec3{0.0f, -glm::length(node.offset)/2, 0.0f});
                }
                glm::mat4 initialBoneTransform = initialRot * initialTrans;
                glm::mat4 T = curTransform * initialBoneTransform;

                if (body.meshes[nodeIdx - 1]) {
                    renderer.queueRender(PhongRenderCommand {
                            body.meshes[nodeIdx - 1],
                            body.materials[nodeIdx - 1],
                            T
                    });
                }

                if (debug) {
                    imRenderer.drawAxisTriad(T, 0.02f, 0.2f, false);
                }
            }

            curTransform = curTransform * glm::translate(node.offset) * glm::mat4_cast(poseState.q[nodeIdx]);
            for (auto childID : node.childJoints) {
                recursionStack.push({childID, curTransform});
            }
        }
    }
}

struct PoseRenderBodyPBR {
    std::vector<Ref<Mesh>> meshes;
    std::vector<Ref<PBRMaterial>> materials;

    static PoseRenderBodyPBR createAsBoxes(const PoseTree& poseTree, float width, Ref<PBRMaterial> material) {
        PoseRenderBodyPBR body;
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

inline void renderMotionClip(PBRenderer& renderer, DebugRenderer& imRenderer,
                             const glmx::pose& poseState, const PoseTree& poseTree, const PoseRenderBodyPBR& body,
                             const glm::mat4& globalTrans = glm::mat4(1.0f), bool debug = false) {

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
                    initialRot = glmx::rotMatrixBetweenVecs(a, b);
                    initialTrans = glm::translate(glm::vec3{0.0f, glm::length(node.offset)/2, 0.0f});
                }
                else {
                    initialRot = glmx::rotMatrixBetweenVecs(a, -b);
                    initialTrans = glm::translate(glm::vec3{0.0f, -glm::length(node.offset)/2, 0.0f});
                }
                glm::mat4 initialBoneTransform = initialRot * initialTrans;
                glm::mat4 T = curTransform * initialBoneTransform;

                if (body.meshes[nodeIdx - 1]) {
                    renderer.queueRender(PBRCommand {
                            body.meshes[nodeIdx - 1],
                            body.materials[nodeIdx - 1],
                            T
                    });
                }

                if (debug) {
                    imRenderer.drawAxisTriad(T, 0.02f, 0.2f, false);
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

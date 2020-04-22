//
// Created by lasagnaphil on 19. 12. 17..
//

#include "PoseRenderBody.h"
#include <glm/gtx/string_cast.hpp>

PoseRenderBody PoseRenderBody::createAsBoxes(const PoseTree& poseTree, float width, Ref<PhongMaterial> material) {
    PoseRenderBody body;
    body.meshes.resize(poseTree.numNodes);
    body.materials.resize(poseTree.numNodes);

    body.meshes[0] = {};
    for (int i = 1; i < poseTree.numNodes; i++) {
        // Don't render rigmesh nodes
        if (poseTree[i].name.find("RIGMESH") != std::string::npos) {
            body.meshes[i] = {};
            continue;
        }
        float offsetLength = glm::length(poseTree[i].offset);
        if (offsetLength > 0) {
            body.meshes[i] = Mesh::makeCube({width, offsetLength, width});
        }
        else {
            body.meshes[i] = {};
        }
        body.materials[i] = material;
    }

    return body;
}

void renderMotionClip(PhongRenderer& renderer, DebugRenderer& imRenderer, glmx::pose_view pose,
                      const PoseTree& poseTree, const PoseRenderBody& body, const glm::mat4& globalTrans, bool debug) {

    // Recursively render all nodes
    std::stack<std::tuple<uint32_t, glm::mat4>> recursionStack;
    recursionStack.push({0, glm::translate(globalTrans, pose.v())});
    while (!recursionStack.empty()) {
        auto [nodeIdx, curTransform] = recursionStack.top();
        recursionStack.pop();

        const PoseTreeNode& node = poseTree[nodeIdx];

        // render bone related to current node (we exclude root node)
        if (nodeIdx != 0) {
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

            if (body.meshes[nodeIdx]) {
                renderer.queueRender(PhongRenderCommand {
                        body.meshes[nodeIdx],
                        body.materials[nodeIdx],
                        T
                });
            }

            if (debug) {
                imRenderer.drawAxisTriad(T, 0.02f, 0.2f, false);
            }
        }

        if (!node.isEndSite()) {
            if (nodeIdx == 0) {
                curTransform = curTransform * glm::mat4_cast(pose.q(nodeIdx));
            }
            else {
                curTransform = curTransform * glm::translate(node.offset) * glm::mat4_cast(pose.q(nodeIdx));
            }
            for (auto childID : node.childJoints) {
                recursionStack.push({childID, curTransform});
            }
        }
    }
}

void
renderMotionClip(PBRenderer& renderer, DebugRenderer& imRenderer, glmx::pose_view pose, const PoseTree& poseTree,
                 const PoseRenderBodyPBR& body, const glm::mat4& globalTrans, bool debug) {

    // Recursively render all nodes
    std::stack<std::tuple<uint32_t, glm::mat4>> recursionStack;
    recursionStack.push({0, glm::translate(globalTrans, pose.v())});
    while (!recursionStack.empty()) {
        auto [nodeIdx, curTransform] = recursionStack.top();
        recursionStack.pop();

        const PoseTreeNode& node = poseTree[nodeIdx];

        // render bone related to current node (we exclude root node)
        if (nodeIdx != 0) {
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

            if (body.meshes[nodeIdx]) {
                renderer.queueRender(PBRCommand {
                        body.meshes[nodeIdx],
                        body.materials[nodeIdx],
                        T
                });
            }

            if (debug) {
                imRenderer.drawAxisTriad(T, 0.02f, 0.2f, false);
            }
        }

        if (!node.isEndSite()) {
            if (nodeIdx == 0) {
                curTransform = curTransform * glm::mat4_cast(pose.q(nodeIdx));
            }
            else {
                curTransform = curTransform * glm::translate(node.offset) * glm::mat4_cast(pose.q(nodeIdx));
            }
            for (auto childID : node.childJoints) {
                recursionStack.push({childID, curTransform});
            }
        }
    }
}

void renderMotionClipComplex(PBRenderer& renderer, DebugRenderer& imRenderer, glmx::pose_view pose,
                             const PoseTree& poseTree, const PoseRenderBodyPBR& body, const glm::mat4& globalTrans,
                             bool debug) {

    // Recursively render all nodes
    std::stack<std::tuple<uint32_t, glm::mat4>> recursionStack;
    recursionStack.push({0, glm::translate(globalTrans, pose.v())});
    while (!recursionStack.empty()) {
        auto [nodeIdx, curTransform] = recursionStack.top();
        recursionStack.pop();

        const PoseTreeNode& node = poseTree[nodeIdx];

        if (nodeIdx != 0) {
            const PoseTreeNode& parentNode = poseTree[node.parent];
            glm::vec3 bodyDir = body.directions[node.parent];
            glm::vec3 bodyOffset = body.offsets[node.parent];
            glm::mat4 initialBoneTransform = glm::translate(bodyOffset + 0.5f * node.offset) * glm::mat4_cast(
                    glmx::quatBetweenVecs(glm::vec3(0, 0, 1), bodyDir));
            glm::mat4 T = curTransform * initialBoneTransform;

            if (body.meshes[node.parent]) {
                renderer.queueRender(PBRCommand{
                        body.meshes[node.parent],
                        body.materials[node.parent],
                        T
                });
            }

            if (debug) {
                imRenderer.drawAxisTriad(T, 0.02f, 0.2f, false);
            }
        }

        if (!node.isEndSite()) {
            if (nodeIdx == 0) {
                curTransform = curTransform * glm::mat4_cast(pose.q(nodeIdx));
            }
            else {
                curTransform = curTransform * glm::translate(node.offset) * glm::mat4_cast(pose.q(nodeIdx));
            }
            for (auto childID : node.childJoints) {
                recursionStack.push({childID, curTransform});
            }
        }
    }
}

PoseRenderBodyPBR PoseRenderBodyPBR::createAsBoxes(const PoseTree& poseTree, float width, Ref<PBRMaterial> material) {
    PoseRenderBodyPBR body;
    body.meshes.resize(poseTree.numNodes);
    body.materials.resize(poseTree.numNodes);
    body.offsets.resize(poseTree.numNodes);
    body.directions.resize(poseTree.numNodes);

    body.meshes[0] = {};
    body.materials[0] = {};
    body.offsets[0] = {};
    body.directions[0] = {};

    for (int i = 1; i < poseTree.numNodes; i++) {
        float offsetLength = glm::length(poseTree[i].offset);
        if (offsetLength > 0) {
            body.meshes[i] = Mesh::makeCube({width, offsetLength, width});
        }
        else {
            body.meshes[i] = {};
        }
        body.materials[i] = material;
        body.offsets[i] = {};
        body.directions[i] = {};
    }

    return body;
}

//
// Created by lasagnaphil on 2019-03-10.
//

#ifndef MOTION_EDITING_POSEDATA_H
#define MOTION_EDITING_POSEDATA_H

#include <string>
#include <vector>
#include <unordered_map>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext.hpp>
#include <span.hpp>
#include <optional>
#include <stack>

#include "GenAllocator.h"
#include "Pose.h"

struct PoseTreeNode {
    std::string name;
    glm::vec3 offset;
    uint32_t parent;
    std::vector<uint32_t> childJoints;
    bool isEndSite = false;
};

struct PoseTree {
    std::vector<PoseTreeNode> allNodes;

    uint32_t numJoints = 0;
    uint32_t numNodes = 0;

    const PoseTreeNode& operator[](uint32_t idx) const { return allNodes[idx]; }
    PoseTreeNode& operator[](uint32_t idx) { return allNodes[idx]; }

    const PoseTreeNode* operator[](const std::string& name) const;

    PoseTreeNode* operator[](const std::string& name) {
        return const_cast<PoseTreeNode*>(const_cast<const PoseTree*>(this)->operator[](name));
    }

    uint32_t findIdx(const std::string& name) const;

    const PoseTreeNode& getRootNode() const {
        return allNodes[0];
    }

    PoseTreeNode& getRootNode() {
        return allNodes[0];
    }

    uint32_t getChildIdx(const PoseTreeNode& node, const std::string& name) const {
        for (uint32_t childIdx : node.childJoints) {
            if (allNodes[childIdx].name == name) {
                return childIdx;
            }
        }
        return (uint32_t)-1;
    }

    uint32_t getChildIdx(const uint32_t nodeIdx, const std::string& name) const {
        for (uint32_t childIdx : allNodes[nodeIdx].childJoints) {
            if (allNodes[childIdx].name == name) {
                return childIdx;
            }
        }
        return (uint32_t)-1;
    }

    const PoseTreeNode& getParentOfNode(const PoseTreeNode& node) const {
        return allNodes[node.parent];
    }

    PoseTreeNode& getParentOfNode(const PoseTreeNode& node) {
        return allNodes[node.parent];
    }

    template <typename Fun>
    void iterateChildrenOfNode(const PoseTreeNode& node, Fun fun) const {
        for (uint32_t childIdx : node.childJoints) {
            fun(allNodes[childIdx]);
        }
    }

    template <typename Fun>
    void iterateChildrenOfNode(PoseTreeNode& node, Fun fun) {
        for (uint32_t childIdx : node.childJoints) {
            fun(allNodes[childIdx]);
        }
    }

    template <class Fun>
    void iterateDFS(Fun fun) {
        std::stack<uint32_t> indices;
        indices.push(0);
        while (!indices.empty()) {
            uint32_t idx = indices.top();
            indices.pop();
            PoseTreeNode& node = allNodes[idx];
            fun(node, idx);
            for (int childIdx : node.childJoints) {
                indices.push(childIdx);
            }
        }
    }
};

struct MotionClipData {
    PoseTree poseTree;
    std::vector<Pose> poseStates;

    // Motion data
    enum class ChannelType : uint8_t {
        Xrot, Yrot, Zrot, Xpos, Ypos, Zpos
    };

    uint32_t numChannels = 0;
    uint32_t numFrames = 0;

    float frameTime;

    static bool loadFromFile(const std::string &filename, MotionClipData &data, float scale = 1.0f);

    void print() const;

    Pose& getFrameState(uint32_t frameIdx) { return poseStates[frameIdx]; }
    const Pose& getFrameState(uint32_t frameIdx) const { return poseStates[frameIdx]; }

    glm::vec3& getRootPos(uint32_t frameIdx) { return poseStates[frameIdx].v; }
    const glm::vec3& getRootPos(uint32_t frameIdx) const { return poseStates[frameIdx].v; }

    glm::quat& getJointRot(uint32_t frameIdx, uint32_t jointIdx) { return poseStates[frameIdx].q[jointIdx]; }
    const glm::quat& getJointRot(uint32_t frameIdx, uint32_t jointIdx) const { return poseStates[frameIdx].q[jointIdx]; }

private:
    void printRecursive(uint32_t jointID, int depth) const;
};

#endif //MOTION_EDITING_POSEDATA_H

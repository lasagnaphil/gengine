//
// Created by lasagnaphil on 19. 11. 12..
//

#ifndef GENGINE_POSETREE_H
#define GENGINE_POSETREE_H

#include <algorithm>
#include <stack>

struct PoseTreeNode {
    std::string name;
    glm::vec3 offset;
    uint32_t parent;
    std::vector<uint32_t> childJoints;
    bool isEndSite = false;

    bool isRoot() const { return parent == 0; }
};

struct PoseTree {
    std::vector<PoseTreeNode> allNodes;

    uint32_t numJoints = 0;
    uint32_t numNodes = 0;

    // Creates a pose tree from a node list.
    // It automatically fills childJoints field, so you don't have to specify it.
    static PoseTree fromNodeList(std::vector<PoseTreeNode>& nodes) {
        PoseTree tree;
        tree.allNodes = nodes;
        for (uint32_t nodeIdx = 1; nodeIdx < nodes.size(); nodeIdx++) {
            tree.numNodes++;
            auto& node = tree[nodeIdx];
            if (node.isEndSite) {
                tree.numJoints++;
                tree[node.parent].childJoints.push_back(nodeIdx);
            }
        }
        return tree;
    }

    const PoseTreeNode& operator[](uint32_t idx) const { return allNodes[idx]; }
    PoseTreeNode& operator[](uint32_t idx) { return allNodes[idx]; }

    const PoseTreeNode* operator[](const std::string& name) const {
        auto it = std::find_if(allNodes.begin(), allNodes.end(), [&](const PoseTreeNode& node) {
            return node.name == name;
        });
        if (it == allNodes.end()) {
            return nullptr;
        }
        else {
            return it.base();
        }
    }

    PoseTreeNode* operator[](const std::string& name) {
        return const_cast<PoseTreeNode*>(const_cast<const PoseTree*>(this)->operator[](name));
    }

    uint32_t findIdx(const std::string& name) const {
        auto it = std::find_if(allNodes.begin(), allNodes.end(), [&](const PoseTreeNode& node) {
            return node.name == name;
        });
        if (it == allNodes.end()) {
            return (uint32_t) -1;
        }
        else {
            return it - allNodes.begin();
        }
    }

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
            for (int i = node.childJoints.size() - 1; i >= 0; i--) {
                indices.push(node.childJoints[i]);
            }
        }
    }
};

#endif //GENGINE_POSETREE_H

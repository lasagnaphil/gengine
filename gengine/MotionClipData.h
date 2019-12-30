//
// Created by lasagnaphil on 2019-03-10.
//

#ifndef MOTION_EDITING_POSEDATA_H
#define MOTION_EDITING_POSEDATA_H

#include "GenAllocator.h"
#include "glmx/pose.h"
#include "PoseTree.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext.hpp>
#include <span.hpp>
#include <stack>

struct MotionClipData {
    PoseTree poseTree;
    std::vector<glmx::pose> poseStates;
    bool valid = false;

    // Motion data
    enum class ChannelType : uint8_t {
        Xrot, Yrot, Zrot, Xpos, Ypos, Zpos
    };

    uint32_t numChannels = 0;
    uint32_t numFrames = 0;

    float frameTime;

    static MotionClipData loadFromFile(const std::string& filename, float scale = 1.0f);

    void print() const;

    glmx::pose& getFrameState(uint32_t frameIdx) { return poseStates[frameIdx]; }
    const glmx::pose& getFrameState(uint32_t frameIdx) const { return poseStates[frameIdx]; }

    glm::vec3& getRootPos(uint32_t frameIdx) { return poseStates[frameIdx].v; }
    const glm::vec3& getRootPos(uint32_t frameIdx) const { return poseStates[frameIdx].v; }

    glm::quat& getJointRot(uint32_t frameIdx, uint32_t jointIdx) { return poseStates[frameIdx].q[jointIdx]; }
    const glm::quat& getJointRot(uint32_t frameIdx, uint32_t jointIdx) const { return poseStates[frameIdx].q[jointIdx]; }

    nonstd::span<glmx::pose> slice(uint32_t start, uint32_t end) {
        assert(start >= 0);
        assert(end <= poseStates.size());
        assert(start < end);
        return nonstd::span<glmx::pose>(poseStates.data() + start, end - start);
    }

    void saveToFile(const std::string& filename, int eulerOrd);

    void switchZtoYup();

private:
    void printRecursive(uint32_t jointID, int depth) const;
    void saveToFileRecursive(uint32_t jointID, std::ostream& ofs, int depth, int eulerOrd);
};

#endif //MOTION_EDITING_POSEDATA_H

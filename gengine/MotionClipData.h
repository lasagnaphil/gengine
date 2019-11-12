//
// Created by lasagnaphil on 2019-03-10.
//

#ifndef MOTION_EDITING_POSEDATA_H
#define MOTION_EDITING_POSEDATA_H

#include "GenAllocator.h"
#include "Pose.h"
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
    std::vector<Pose> poseStates;
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

    Pose& getFrameState(uint32_t frameIdx) { return poseStates[frameIdx]; }
    const Pose& getFrameState(uint32_t frameIdx) const { return poseStates[frameIdx]; }

    glm::vec3& getRootPos(uint32_t frameIdx) { return poseStates[frameIdx].v; }
    const glm::vec3& getRootPos(uint32_t frameIdx) const { return poseStates[frameIdx].v; }

    glm::quat& getJointRot(uint32_t frameIdx, uint32_t jointIdx) { return poseStates[frameIdx].q[jointIdx]; }
    const glm::quat& getJointRot(uint32_t frameIdx, uint32_t jointIdx) const { return poseStates[frameIdx].q[jointIdx]; }

    void saveToFile(const std::string& filename);

private:
    void printRecursive(uint32_t jointID, int depth) const;
    void saveToFileRecursive(uint32_t jointID, std::ostream& ofs, int depth);
};

#endif //MOTION_EDITING_POSEDATA_H

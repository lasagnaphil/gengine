//
// Created by lasagnaphil on 2019-03-10.
//

#ifndef MOTION_EDITING_POSEDATA_H
#define MOTION_EDITING_POSEDATA_H

#include "Arena.h"
#include "glmx/pose.h"
#include "PoseTree.h"
#include "anim/MotionClip.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext.hpp>
#include <span.hpp>
#include <stack>

struct BVHData {
    PoseTree poseTree;
    MotionClip clip;

    // Motion data
    enum class ChannelType : uint8_t {
        Xrot, Yrot, Zrot, Xpos, Ypos, Zpos
    };

    float frameTime;

    static bool loadFromFile(const std::string& filename, BVHData& bvhData, float scale = 1.0f);

    void print() const;

    void saveToFile(const std::string& filename, int eulerOrd);

    void switchZtoYup();

    void removeJoint(uint32_t nodeIdx);

    void removeJoint(const std::string& nodeName);

    void removeCMUPhantomJoints();

    bool checkValidity();

    void moveStartingRoot(glm::vec3 pos);

    void moveStartingRoot(glmx::transform t);

    glmx::pose samplePose(float time);

private:
    void printRecursive(uint32_t jointID, int depth) const;
    void saveToFileRecursive(uint32_t jointID, std::ostream& ofs, int depth, int eulerOrd);
};

#endif //MOTION_EDITING_POSEDATA_H

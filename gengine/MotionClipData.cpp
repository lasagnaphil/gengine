//
// Created by lasagnaphil on 2019-03-10.
//

#include "MotionClipData.h"

#include <iomanip>
#include <fstream>
#include <sstream>
#include <iostream>
#include <optional>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

std::optional<MotionClipData::ChannelType> stringToChannelType(const std::string& name) {
    switch (name[0]) {
        case 'X':
            if (name == "Xposition") return MotionClipData::ChannelType::Xpos;
            else if (name == "Xrotation") return MotionClipData::ChannelType::Xrot;
            else return {};
        case 'Y':
            if (name == "Yposition") return MotionClipData::ChannelType::Ypos;
            else if (name == "Yrotation") return MotionClipData::ChannelType::Yrot;
            else return {};
        case 'Z':
            if (name == "Zposition") return MotionClipData::ChannelType::Zpos;
            else if (name == "Zrotation") return MotionClipData::ChannelType::Zrot;
            else return {};
        default:
            return {};
    }
}

MotionClipData MotionClipData::loadFromFile(const std::string& filename, float scale) {
    MotionClipData data;
    data.valid = true;

    std::ifstream file(filename);

    std::string line;
    std::string keyword;

    enum class ParseState { Root, Joint, EndSite, Motion };
    ParseState state = ParseState::Root;
    int lineCount = 0;

    uint32_t curJointID = {};
    uint32_t childJointID = {};

    std::istringstream iss;
    auto newLine = [&]() -> std::istringstream& {
        std::getline(file, line);
        lineCount++;
        iss = std::istringstream(line);
        return iss;
    };

    // temporary storage for end sites
    std::vector<PoseTreeNode> endSites;

    // type of each channel
    std::vector<ChannelType> channelTypeData;

    bool finished = false;
    bool error = false;
    while (!error && !finished) {
        switch (state) {
            case ParseState::Root: {
                PoseTreeNode curJoint;
                curJointID = 0; // root joint id

                curJoint.parent = 0;

                newLine() >> keyword;

                newLine() >> keyword >> curJoint.name;

                newLine() >> keyword;
                if (keyword != "{") {
                    error = true;
                    break;
                }
                newLine() >> keyword >> curJoint.offset.x >> curJoint.offset.y >> curJoint.offset.z; // OFFSET
                curJoint.offset *= scale;
                int channels;
                std::string channelNames[6];
                newLine() >> keyword >> channels
                          >> channelNames[0] >> channelNames[1] >> channelNames[2]
                          >> channelNames[3] >> channelNames[4] >> channelNames[5];
                if (channels != 6) {
                    std::cerr << "Only root node with 6 channels supported!" << std::endl;
                    error = true;
                    break;
                }
                for (int i = 0; i < 6; i++) {
                    if (auto channelType = stringToChannelType(channelNames[i])) {
                        channelTypeData.push_back(*channelType);
                    }
                    else {
                        std::cerr << "Invalid channel type." << std::endl;
                        error = true;
                        break;
                    }
                }
                data.numChannels += 6; data.poseTree.numJoints++; data.poseTree.numNodes++; // CHANNELS
                childJointID = curJointID;

                newLine() >> keyword;
                if (keyword == "JOINT") {
                    state = ParseState::Joint;
                }
                else if (keyword == "End") {
                    state = ParseState::EndSite;
                }
                data.poseTree.allNodes.push_back(curJoint);
                break;
            }
            case ParseState::Joint: {
                PoseTreeNode childJoint;
                childJointID = data.poseTree.allNodes.size();
                PoseTreeNode& curJoint = data.poseTree.allNodes[curJointID];

                iss >> childJoint.name;
                newLine() >> keyword;
                if (keyword != "{") {
                    error = true;
                    break;
                }
                newLine() >> keyword >> childJoint.offset.x >> childJoint.offset.y
                          >> childJoint.offset.z; // OFFSET
                childJoint.offset *= scale;

                int channels;
                std::vector<std::string> channelNames;
                newLine() >> keyword >> channels;
                if (channels > 0 && channels <= 6) {
                    channelNames.resize(channels);
                }
                else {
                    fprintf(stderr, "Invalid number of channels. (%d)", channels);
                    error = true;
                    break;
                }
                for (int c = 0; c < channels; c++) {
                    iss >> channelNames[c];
                }
                for (int c = 0; c < channels; c++) {
                    if (auto channelType = stringToChannelType(channelNames[c])) {
                        channelTypeData.push_back(*channelType);
                    }
                    else {
                        fprintf(stderr, "Invalid channel type.");
                        error = true;
                        break;
                    }
                }
                if (error) break;
                data.numChannels += channels; data.poseTree.numJoints++; data.poseTree.numNodes++; // CHANNELS
                curJoint.childJoints.push_back(childJointID);
                childJoint.parent = curJointID;
                curJointID = childJointID;

                newLine() >> keyword;
                if (keyword == "JOINT") {
                    state = ParseState::Joint;
                }
                else if (keyword == "End") {
                    state = ParseState::EndSite;
                }
                data.poseTree.allNodes.push_back(childJoint);
                break;
            }
            case ParseState::EndSite: {
                PoseTreeNode childJoint;
                curJointID = childJointID;
                uint32_t endSiteID = endSites.size();
                PoseTreeNode& curJoint = data.poseTree.allNodes[curJointID];

                childJoint.name = "End Site";
                newLine() >> keyword;
                if (keyword != "{") {
                    error = true;
                    break;
                }
                newLine() >> keyword >> childJoint.offset.x >> childJoint.offset.y >> childJoint.offset.z;
                childJoint.offset *= scale;
                data.poseTree.numNodes++;
                // The most significant bit tells if the id is an end site or not
                curJoint.childJoints.push_back(endSiteID + (1 << 31));
                childJoint.parent = curJointID;
                curJointID = childJointID;

                newLine() >> keyword;
                if (keyword != "}") {
                    error = true;
                    break;
                }
                newLine() >> keyword;
                while (keyword == "}") {
                    childJointID = curJointID;
                    curJointID = data.poseTree.allNodes[curJointID].parent;
                    newLine() >> keyword;
                }
                if (keyword == "JOINT") {
                    state = ParseState::Joint;
                }
                else if (keyword == "MOTION") {
                    state = ParseState::Motion;
                }

                endSites.push_back(childJoint);
                break;
            }
            case ParseState::Motion: {
                newLine() >> keyword >> data.numFrames;
                std::string _;
                newLine() >> keyword >> _ >> data.frameTime;

                data.poseStates.resize(data.numFrames);
                float num;
                glm::quat rot = glm::identity<glm::quat>();
                int rotCount = 0;
                for (int f = 0; f < data.numFrames; f++) {
                    auto& poseState = data.poseStates[f];
                    newLine();
                    for (int c = 0; c < data.numChannels; c++) {
                        iss >> num;
                        if (!iss) {
                            error = true;
                            break;
                        }
                        if (c < 3) {
                            poseState.v[c] = num * scale;
                        }
                        else {
                            switch (channelTypeData[c]) {
                                case ChannelType::Xrot:
                                    rot = glm::rotate(rot, glm::radians(num), {1, 0, 0});
                                    rotCount++;
                                    break;
                                case ChannelType::Yrot:
                                    rot = glm::rotate(rot, glm::radians(num), {0, 1, 0});
                                    rotCount++;
                                    break;
                                case ChannelType::Zrot:
                                    rot = glm::rotate(rot, glm::radians(num), {0, 0, 1});
                                    rotCount++;
                                    break;
                                default:
                                    // Ignore Xposition, Yposition, Zposition
                                    break;
                            }
                            if (rotCount == 3) {
                                poseState.q.push_back(rot);
                                rot = glm::identity<glm::quat>();
                                rotCount = 0;
                            }
                        }
                    }
                }
                finished = true;
                break;
            }
        }

    }

    if (error) {
        data.valid = false;
        return data;
    }

    // Now we combine the joint data and end site data into one std::vector.
    // Body node indices need to be in 0...numJoints-1,
    // and end site indices need to be in numJoints...numNodes-1.
    // So need to make sure the highest bit of the end site ids are set back to zero,
    // and then add numJoints to set the index in the right location.
    data.poseTree.allNodes.reserve(data.poseTree.allNodes.size() + endSites.size());
    data.poseTree.allNodes.insert(data.poseTree.allNodes.end(), endSites.begin(), endSites.end());
    for (uint32_t i = 0; i < data.poseTree.numJoints; i++) {
        auto& joint = data.poseTree.allNodes[i];
        for (auto& childJointID : joint.childJoints) {
            if (childJointID & (1 << 31)) {
                childJointID = (childJointID & ~(1 << 31)) + data.poseTree.numJoints;
            }
        }
    }

    data.poseTree.constructNodeNameMapping();

    return data;
}

void MotionClipData::print() const {
    printRecursive(0 /* root joint id */, 0);
}

void MotionClipData::printRecursive(uint32_t jointID, int depth) const {
    const PoseTreeNode& joint = poseTree.allNodes[jointID];
    if (!joint.isEndSite()) {
        for (int i = 0; i < depth; i++) { std::cout << "    "; }
        std::cout << "Name: " << joint.name << std::endl;
        for (int i = 0; i < depth; i++) { std::cout << "    "; }
        std::cout << "Offset: " << joint.offset.x << " " << joint.offset.y << " " << joint.offset.z << std::endl;
        for (auto child : joint.childJoints) {
            printRecursive(child, depth + 1);
        }
    }
    else {
        for (int i = 0; i < depth; i++) { std::cout << "    "; }
        std::cout << "End Site Offset: " << joint.offset.x << " " << joint.offset.y << " " << joint.offset.z << std::endl;
    }
}

void MotionClipData::saveToFile(const std::string& filename, int eulerOrd) {
    using std::endl;
    std::ofstream ofs(filename);
    ofs << std::fixed;
    ofs.precision(6);

    saveToFileRecursive(0, ofs, 0, eulerOrd);
    ofs << "MOTION" << endl;
    ofs << "Frames: " << numFrames << endl;
    ofs << "Frame Time: " << frameTime << endl;
    for (int f = 0; f < numFrames; f++) {
        glmx::pose& pose = poseStates[f];
        ofs << pose.v.x << " " << pose.v.y << " " << pose.v.z << " ";
        for (int i = 0; i < pose.size(); i++) {
            glm::vec3 e = glmx::quatToEuler(pose.q[i], eulerOrd);
            switch (eulerOrd) {
                case EulOrdXYZr:
                case EulOrdXYXr:
                case EulOrdXZYr:
                case EulOrdXZXr:
                case EulOrdYZXr:
                case EulOrdYZYr:
                case EulOrdYXZr:
                case EulOrdYXYr:
                case EulOrdZXYr:
                case EulOrdZXZr:
                case EulOrdZYXr:
                case EulOrdZYZr:
                    ofs << glm::degrees(e.x) << " " << glm::degrees(e.y) << " " << glm::degrees(e.z) << " "; break;
                case EulOrdXYZs:
                case EulOrdXYXs:
                case EulOrdXZYs:
                case EulOrdXZXs:
                case EulOrdYZXs:
                case EulOrdYZYs:
                case EulOrdYXZs:
                case EulOrdYXYs:
                case EulOrdZXYs:
                case EulOrdZXZs:
                case EulOrdZYXs:
                case EulOrdZYZs:
                    ofs << glm::degrees(e.z) << " " << glm::degrees(e.y) << " " << glm::degrees(e.x) << " "; break;
            }
        }
        ofs << endl;
    }
}

void MotionClipData::saveToFileRecursive(uint32_t jointIdx, std::ostream& ofs, int depth, int eulerOrd) {
    using std::endl;
    PoseTreeNode& node = poseTree[jointIdx];

    std::string tabs;
    for (int i = 0; i < depth; i++) {
        tabs += "\t";
    }
    if (jointIdx == 0) {
        ofs << tabs << "HIERARCHY" << endl;
        ofs << tabs << "ROOT " << node.name << endl;
        ofs << tabs << "{" << endl;
        ofs << tabs << "\tOFFSET " << node.offset.x << " " << node.offset.y << " " << node.offset.z << endl;
        ofs << tabs << "\tCHANNELS 6 Xposition Yposition Zposition";
        switch (eulerOrd) {
            case EulOrdXYZr: ofs << " Xrotation Yrotation Zrotation" << endl; break;
            case EulOrdXYXr: ofs << " Xrotation Yrotation Xrotation" << endl; break;
            case EulOrdXZYr: ofs << " Xrotation Zrotation Yrotation" << endl; break;
            case EulOrdXZXr: ofs << " Xrotation Zrotation Xrotation" << endl; break;
            case EulOrdYZXr: ofs << " Yrotation Zrotation Xrotation" << endl; break;
            case EulOrdYZYr: ofs << " Yrotation Zrotation Yrotation" << endl; break;
            case EulOrdYXZr: ofs << " Yrotation Xrotation Zrotation" << endl; break;
            case EulOrdYXYr: ofs << " Yrotation Xrotation Yrotation" << endl; break;
            case EulOrdZXYr: ofs << " Zrotation Xrotation Yrotation" << endl; break;
            case EulOrdZXZr: ofs << " Zrotation Xrotation Zrotation" << endl; break;
            case EulOrdZYXr: ofs << " Zrotation Yrotation Xrotation" << endl; break;
            case EulOrdZYZr: ofs << " Zrotation Yrotation Zrotation" << endl; break;
            case EulOrdXYZs: ofs << " Zrotation Yrotation Xrotation" << endl; break;
            case EulOrdXYXs: ofs << " Xrotation Yrotation Xrotation" << endl; break;
            case EulOrdXZYs: ofs << " Yrotation Zrotation Xrotation" << endl; break;
            case EulOrdXZXs: ofs << " Xrotation Zrotation Xrotation" << endl; break;
            case EulOrdYZXs: ofs << " Xrotation Zrotation Yrotation" << endl; break;
            case EulOrdYZYs: ofs << " Yrotation Zrotation Yrotation" << endl; break;
            case EulOrdYXZs: ofs << " Zrotation Xrotation Yrotation" << endl; break;
            case EulOrdYXYs: ofs << " Yrotation Xrotation Yrotation" << endl; break;
            case EulOrdZXYs: ofs << " Yrotation Xrotation Zrotation" << endl; break;
            case EulOrdZXZs: ofs << " Zrotation Xrotation Zrotation" << endl; break;
            case EulOrdZYXs: ofs << " Xrotation Yrotation Zrotation" << endl; break;
            case EulOrdZYZs: ofs << " Zrotation Yrotation Zrotation" << endl; break;
        }
    }
    else if (!node.isEndSite()) {
        ofs << tabs << "JOINT " << node.name << endl;
        ofs << tabs << "{" << endl;
        ofs << tabs << "\tOFFSET " << node.offset.x << " " << node.offset.y << " " << node.offset.z << endl;
        ofs << tabs << "\tCHANNELS 3";
        switch (eulerOrd) {
            case EulOrdXYZr: ofs << " Xrotation Yrotation Zrotation" << endl; break;
            case EulOrdXYXr: ofs << " Xrotation Yrotation Xrotation" << endl; break;
            case EulOrdXZYr: ofs << " Xrotation Zrotation Yrotation" << endl; break;
            case EulOrdXZXr: ofs << " Xrotation Zrotation Xrotation" << endl; break;
            case EulOrdYZXr: ofs << " Yrotation Zrotation Xrotation" << endl; break;
            case EulOrdYZYr: ofs << " Yrotation Zrotation Yrotation" << endl; break;
            case EulOrdYXZr: ofs << " Yrotation Xrotation Zrotation" << endl; break;
            case EulOrdYXYr: ofs << " Yrotation Xrotation Yrotation" << endl; break;
            case EulOrdZXYr: ofs << " Zrotation Xrotation Yrotation" << endl; break;
            case EulOrdZXZr: ofs << " Zrotation Xrotation Zrotation" << endl; break;
            case EulOrdZYXr: ofs << " Zrotation Yrotation Xrotation" << endl; break;
            case EulOrdZYZr: ofs << " Zrotation Yrotation Zrotation" << endl; break;
            case EulOrdXYZs: ofs << " Zrotation Yrotation Xrotation" << endl; break;
            case EulOrdXYXs: ofs << " Xrotation Yrotation Xrotation" << endl; break;
            case EulOrdXZYs: ofs << " Yrotation Zrotation Xrotation" << endl; break;
            case EulOrdXZXs: ofs << " Xrotation Zrotation Xrotation" << endl; break;
            case EulOrdYZXs: ofs << " Xrotation Zrotation Yrotation" << endl; break;
            case EulOrdYZYs: ofs << " Yrotation Zrotation Yrotation" << endl; break;
            case EulOrdYXZs: ofs << " Zrotation Xrotation Yrotation" << endl; break;
            case EulOrdYXYs: ofs << " Yrotation Xrotation Yrotation" << endl; break;
            case EulOrdZXYs: ofs << " Yrotation Xrotation Zrotation" << endl; break;
            case EulOrdZXZs: ofs << " Zrotation Xrotation Zrotation" << endl; break;
            case EulOrdZYXs: ofs << " Xrotation Yrotation Zrotation" << endl; break;
            case EulOrdZYZs: ofs << " Zrotation Yrotation Zrotation" << endl; break;
        }
    }
    else {
        ofs << tabs << "End Site" << endl;
        ofs << tabs << "{" << endl;
        ofs << tabs << "\tOFFSET " << node.offset.x << " " << node.offset.y << " " << node.offset.z << endl;
    }
    for (uint32_t childIdx : node.childJoints) {
        saveToFileRecursive(childIdx, ofs, depth + 1, eulerOrd);
    }
    ofs << tabs << "}" << endl;
}

void MotionClipData::switchZtoYup() {
    for (auto& pose : poseStates) {
        pose.v = glmx::Rx(-M_PI/2) * pose.v;
        for (int i = 0; i < pose.q.size(); i++) {
            pose.q[i] = glmx::Rx(-M_PI/2) * pose.q[i] * glmx::Rx(M_PI/2);
        }
    }
    for (auto& node : poseTree.allNodes) {
        node.offset = glmx::Rx(-M_PI/2) * node.offset;
    }
}

void MotionClipData::removeJoint(uint32_t nodeIdx) {
    if (nodeIdx == (uint32_t)-1) {
        fprintf(stderr, "Tring to remove invalid joint!\n");
    }
    assert(poseTree[nodeIdx].childJoints.size() == 1 && "Only joints with one child can be removed");
    uint32_t parentIdx = poseTree[nodeIdx].parent;
    uint32_t childIdx = poseTree[nodeIdx].childJoints[0];

    auto& node = poseTree[nodeIdx];
    auto& parentNode = poseTree[parentIdx];
    auto& childNode = poseTree[childIdx];
    std::string nodeName = node.name;

    poseTree[childIdx].parent = parentIdx;

    for (uint32_t& i : parentNode.childJoints) {
        if (i == nodeIdx) {
            i = childIdx;
            break;
        }
    }

    poseTree.numJoints--;
    poseTree.numNodes--;
    poseTree.allNodes.erase(poseTree.allNodes.begin() + nodeIdx, poseTree.allNodes.begin() + nodeIdx+1);

    for (auto& pose : poseStates) {
        glm::quat q = pose.q[nodeIdx];
        pose.q[childIdx] = q * pose.q[childIdx];
        pose.q.erase(pose.q.begin() + nodeIdx, pose.q.begin() + nodeIdx+1);
    }

    for (auto& node : poseTree.allNodes) {
        if (node.parent > nodeIdx) {
            node.parent--;
        }
        for (uint32_t& i : node.childJoints) {
            if (i > nodeIdx) {
                i--;
            }
        }
    }

    poseTree.nodeNameMap.erase(nodeName);
    for (auto& [key, i] : poseTree.nodeNameMap) {
        if (i > nodeIdx) {
            i--;
        }
    }
}

void MotionClipData::removeJoint(const std::string& nodeName) {
    uint32_t nodeIdx = poseTree.findIdx(nodeName);
    if (nodeIdx != (uint32_t)-1) {
        removeJoint(nodeIdx);
    }
    else {
        fprintf(stderr, "Trying to remove invalid joint %s!\n", nodeName.c_str());
    }
}

void MotionClipData::removeCMUPhantomJoints() {
    removeJoint("LHipJoint");
    removeJoint("RHipJoint");
    removeJoint("LowerBack");
    removeJoint("Neck");
    removeJoint("LeftShoulder");
    removeJoint("LeftFingerBase");
    removeJoint("LThumb");
    removeJoint("RightShoulder");
    removeJoint("RightFingerBase");
    removeJoint("RThumb");
}

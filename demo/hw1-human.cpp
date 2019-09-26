//
// Created by lasagnaphil on 19. 9. 23..
//

#include <InputManager.h>
#include "App.h"
#include "PhongRenderer.h"
#include "GizmosRenderer.h"
#include "FlyCamera.h"
#include "Pose.h"
#include "PoseRenderBody.h"

#include <map>
#include <glm/gtx/euler_angles.hpp>

struct PoseAnimation {
    std::map<uint32_t, Pose> poseKeyframes;

    void insertFrame(uint32_t frameIdx, const Pose& pose) {
        poseKeyframes.insert({frameIdx, pose});
    }

    Pose getPoseAtFrame(uint32_t frameIdx) {
        if (poseKeyframes.count(frameIdx) == 1) {
            return poseKeyframes[frameIdx];
        }
        else {
            for (auto it = poseKeyframes.begin(); it != poseKeyframes.end(); it++) {
                auto i1 = it->first;
                auto i2 = std::next(it)->first;
                if (frameIdx > i1 && frameIdx < i2) {
                    Pose p1 = it->second;
                    Pose p2 = std::next(it)->second;
                    float t = (float)(frameIdx - i1) / (float)(i2 - i1);
                    return slerp(p1, p2, t);
                }
            }
            return Pose::empty(poseKeyframes.begin()->second.size());
        }
    }

    std::size_t length() {
        return poseKeyframes.rbegin()->first;
    }
};

class MyApp : public App {
public:
    MyApp() : App(true) {}

    void loadResources() override {
        FlyCamera* camera = initCamera<FlyCamera>();
        camera->transform->setPosition({0.0f, 1.0f, 2.0f});

        Ref<Image> checkerImage = Resources::make<Image>("resources/textures/checker.png");
        Ref<Texture> planeTexture = Texture::fromImage(checkerImage);
        checkerImage.release();

        groundMat = Resources::make<Material>();
        groundMat->ambient = {0.1f, 0.1f, 0.1f, 1.0f};
        groundMat->specular = {0.7f, 0.7f, 0.7f, 1.0f};
        groundMat->shininess = 32.0f;
        groundMat->texDiffuse = planeTexture;
        groundMat->texSpecular = {};

        groundMesh = Mesh::makePlane(1000.0f, 100.0f);

        // Load BVH file, only copy the tree structure of the human
        MotionClipData tmpBvh;
        MotionClipData::loadFromFile("resources/cmu_07_02_1.bvh", tmpBvh);
        poseTree = tmpBvh.poseTree;

        for (auto& node : poseTree.allNodes) {
            node.offset *= 0.01f;
        }

        // Create empty pose
        Pose basePose = Pose::empty(poseTree.numJoints);
        basePose.v.y = 1.05f;

        Pose p1 = basePose, p2 = basePose, p3 = basePose, p4 = basePose;

        auto LHipJointIdx = poseTree.findIdx("LHipJoint");
        auto RHipJointIdx = poseTree.findIdx("RHipJoint");
        auto LeftArmIdx = poseTree.findIdx("LeftArm");
        auto RightArmIdx = poseTree.findIdx("RightArm");

        p1.q[LHipJointIdx] = glm::rotate(basePose.q[LHipJointIdx],
                                               glm::radians(-20.0f), {0, 0, 1});

        p1.q[RHipJointIdx] = glm::rotate(basePose.q[RHipJointIdx],
                                               glm::radians(20.0f), {0, 0, 1});

        p1.q[LeftArmIdx] = glm::rotate(basePose.q[LeftArmIdx],
                                             glm::radians(-30.0f), {0, 0, 1});

        p1.q[RightArmIdx] = glm::rotate(basePose.q[RightArmIdx],
                                              glm::radians(30.0f), {0, 0, 1});

        p2.q[LHipJointIdx] = glm::rotate(basePose.q[LHipJointIdx],
                glm::radians(50.0f), {0, 0, 1});

        p2.q[RHipJointIdx] = glm::rotate(basePose.q[RHipJointIdx],
                glm::radians(-50.0f), {0, 0, 1});

        p2.q[LeftArmIdx] = glm::rotate(basePose.q[LeftArmIdx],
                glm::radians(-30.0f), {0, 0, 1});

        p2.q[RightArmIdx] = glm::rotate(basePose.q[RightArmIdx],
                glm::radians(30.0f), {0, 0, 1});

        poseAnim.insertFrame(0, p1);
        poseAnim.insertFrame(10, p2);
        poseAnim.insertFrame(20, p3);

        currentPose = poseAnim.getPoseAtFrame(0);

        // Material of human
        Ref<Material> bodyMat = Resources::make<Material>();
        bodyMat->ambient = {0.1f, 0.1f, 0.1f, 1.0f};
        bodyMat->specular = {0.7f, 0.7f, 0.7f, 1.0f};
        bodyMat->diffuse = {1.0f, 0.0f, 0.0f, 1.0f};
        bodyMat->shininess = 64.0f;
        bodyMat->texDiffuse = {};
        bodyMat->texSpecular = {};

        // Create body using capsules
        poseRenderBody = PoseRenderBody::createAsBoxes(poseTree, 0.05f, bodyMat);
    }

    void processInput(SDL_Event &event) override {
    }

    void update(float dt) override {
        auto inputMgr = InputManager::get();
        if (inputMgr->isKeyEntered(SDL_SCANCODE_1)) {
            phongRenderer.viewDepthBufferDebug = !phongRenderer.viewDepthBufferDebug;
        }
        currentPose = poseAnim.getPoseAtFrame(frameIdx);
    }

    void render() override {
        phongRenderer.queueRender({groundMesh, groundMat, rootTransform->getWorldTransform()});
        renderMotionClip(phongRenderer, currentPose, poseTree, poseRenderBody);

        phongRenderer.render();
        gizmosRenderer.render();

        ImGui::Begin("Human Control");
        ImGui::SliderInt("Frame Idx", &frameIdx, 0, poseAnim.length());
        Pose pose = poseAnim.getPoseAtFrame(frameIdx);
        for (uint32_t i = 0; i < poseTree.numJoints; i++) {
            auto& node = poseTree[i];
            glm::vec3 v = glm::eulerAngles(currentPose.q[i]);
            ImGui::SliderFloat3(node.name.c_str(), (float*)&v, -M_PIf32, M_PIf32);
            currentPose.q[i] = glm::rotate(glm::rotate(glm::rotate(
                    glm::identity<glm::quat>(), v.x, {1, 0, 0}), v.y, {0, 1, 0}), v.z, {0, 0, 1});
        }
        ImGui::End();
    }

    void release() override {
    }

private:
    PoseAnimation poseAnim;
    Pose currentPose;

    PoseTree poseTree;
    PoseRenderBody poseRenderBody;

    int frameIdx = 0;

    Ref<Material> groundMat;
    Ref<Mesh> groundMesh;
};

int main(int argc, char** argv) {
    MyApp app;
    app.start();

    return 0;
}
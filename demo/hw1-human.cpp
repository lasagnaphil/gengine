//
// Created by lasagnaphil on 19. 9. 23..
//

#include "gengine/InputManager.h"
#include "gengine/App.h"
#include "gengine/PhongRenderer.h"
#include "gengine/FlyCamera.h"
#include "glmx/pose.h"
#include "gengine/PoseRenderBody.h"

#include <map>
#include <glm/gtx/euler_angles.hpp>
#include <tinyxml2.h>


struct PoseAnimation {
    std::map<uint32_t, glmx::pose> poseKeyframes;

    void insertFrame(uint32_t frameIdx, const glmx::pose& pose) {
        poseKeyframes.insert({frameIdx, pose});
    }

    glmx::pose getPoseAtFrame(uint32_t frameIdx) {
        if (poseKeyframes.count(frameIdx) == 1) {
            return poseKeyframes[frameIdx];
        }
        else {
            for (auto it = poseKeyframes.begin(); it != poseKeyframes.end(); it++) {
                auto i1 = it->first;
                auto i2 = std::next(it)->first;
                if (frameIdx > i1 && frameIdx < i2) {
                    glmx::pose p1 = it->second;
                    glmx::pose p2 = std::next(it)->second;
                    float t = (float)(frameIdx - i1) / (float)(i2 - i1);
                    glmx::pose p = p1;
                    slerp(p1, p2, t, p);
                    return p;
                }
            }
            return glmx::pose::empty(poseKeyframes.begin()->second.size());
        }
    }

    std::size_t length() {
        return poseKeyframes.rbegin()->first;
    }

};


class MyApp : public App {
public:
    MyApp() : App() {}

    void loadResources() override {
        FlyCamera* camera = dynamic_cast<FlyCamera*>(this->camera.get());
        camera->transform->setPosition({0.0f, 1.0f, 2.0f});

        Ref<Image> checkerImage = Image::fromFile("resources/textures/checker.png");
        Ref<Texture> planeTexture = Texture::fromImage(checkerImage);
        checkerImage.release();

        groundMat = Resources::make<PhongMaterial>();
        groundMat->ambient = {0.1f, 0.1f, 0.1f, 1.0f};
        groundMat->specular = {0.7f, 0.7f, 0.7f, 1.0f};
        groundMat->shininess = 32.0f;
        groundMat->texDiffuse = planeTexture;
        groundMat->texSpecular = {};

        groundMesh = Mesh::makePlane(1000.0f, 100.0f);

        // Load BVH file, only copy the tree structure of the human
        BVHData tmpBvh;
        if (!BVHData::loadFromFile("resources/cmu_07_02_1.bvh", tmpBvh, 0.01f)) {
            fprintf(stderr, "Cannot load BVH!\n");
            exit(EXIT_FAILURE);
        }
        tmpBvh.removeJoint("LHipJoint");
        tmpBvh.removeJoint("RHipJoint");
        tmpBvh.removeJoint("LowerBack");
        tmpBvh.removeJoint("Neck");
        tmpBvh.removeJoint("LeftFingerBase");
        tmpBvh.removeJoint("LThumb");
        tmpBvh.removeJoint("RightFingerBase");
        tmpBvh.removeJoint("RThumb");

        poseTree = tmpBvh.poseTree;

        // Create empty pose
        glmx::pose basePose = glmx::pose::empty(poseTree.numJoints);
        basePose.v().y = 1.05f;

        glmx::pose p1 = basePose, p2 = basePose, p3 = basePose, p4 = basePose;

        auto LHipJointIdx = poseTree.findIdx("LeftUpLeg");
        auto RHipJointIdx = poseTree.findIdx("RightUpLeg");
        auto LeftShoulderIdx = poseTree.findIdx("LeftShoulder");
        auto RightShoulderIdx = poseTree.findIdx("RightShoulder");
        auto LeftArmIdx = poseTree.findIdx("LeftArm");
        auto RightArmIdx = poseTree.findIdx("RightArm");

        auto Zrot = [](float z) -> glm::quat {
            return glm::rotate(glm::identity<glm::quat>(), glm::radians(z), {0, 0, 1});
        };

        p1.q(LHipJointIdx) = Zrot(-5.0f);
        p1.q(RHipJointIdx) = Zrot(5.0f);

        p2.q(LeftShoulderIdx) = Zrot(-40.0f);
        p2.q(RightShoulderIdx) = Zrot(40.0f);
        p2.q(LeftArmIdx) = Zrot(-40.0f);
        p2.q(RightArmIdx) = Zrot(40.0f);
        p2.q(LHipJointIdx) = Zrot(-15.0f);
        p2.q(RHipJointIdx) = Zrot(15.0f);

        p3.q(LeftShoulderIdx) = Zrot(20.0f);
        p3.q(RightShoulderIdx) = Zrot(-20.0f);
        p3.q(LeftArmIdx) = Zrot(90.0f);
        p3.q(RightArmIdx) = Zrot(-90.0f);

        p4 = p2;

        glmx::pose p15 = p1, p25 = p1, p35 = p1, p45 = p1;

        glmx::slerp(p1, p2, 0.5f, p15);
        p15.v().y += 0.3f;
        glmx::slerp(p2, p3, 0.5f, p25);
        p25.v().y += 0.3f;
        glmx::slerp(p3, p4, 0.5f, p35);
        p35.v().y += 0.3f;
        glmx::slerp(p4, p1, 0.5f, p45);
        p45.v().y += 0.3f;

        poseAnim.insertFrame(0*12, p1);
        poseAnim.insertFrame(1*12, p15);
        poseAnim.insertFrame(2*12, p2);
        poseAnim.insertFrame(3*12, p25);
        poseAnim.insertFrame(4*12, p3);
        poseAnim.insertFrame(5*12, p35);
        poseAnim.insertFrame(6*12, p4);
        poseAnim.insertFrame(7*12, p45);
        poseAnim.insertFrame(8*12, p1);

        currentPose = poseAnim.getPoseAtFrame(0);

        // Material of human
        Ref<PhongMaterial> bodyMat = Resources::make<PhongMaterial>();
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
        static float time = 0.0f;
        time += dt;
        auto inputMgr = InputManager::get();
        if (inputMgr->isKeyEntered(SDL_SCANCODE_1)) {
            phongRenderer.viewDepthBufferDebug = !phongRenderer.viewDepthBufferDebug;
        }
        if (isPlaying) {
            while (time >= 1.0f / 60.0f) {
                frameIdx = (frameIdx + 1) % poseAnim.length();
                currentPose = poseAnim.getPoseAtFrame(frameIdx);
                time -= 1.0f / 60.0f;
            }
        }
    }

    void render() override {
        phongRenderer.queueRender({groundMesh, groundMat, rootTransform->getWorldTransform()});
        renderMotionClip(phongRenderer, imRenderer, currentPose.getView(), poseTree, poseRenderBody);

        phongRenderer.render();
        gizmosRenderer.render();

        ImGui::SetNextWindowPos(ImVec2(60, 150), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(500, 900), ImGuiCond_FirstUseEver);
        ImGui::Begin("Human Control");

        if (ImGui::SliderInt("Frame Idx", &frameIdx, 0, poseAnim.length())) {
            currentPose = poseAnim.getPoseAtFrame(frameIdx);
        }
        if (ImGui::Button("Play / Pause")) {
            isPlaying = !isPlaying;
        }
        glmx::pose pose = poseAnim.getPoseAtFrame(frameIdx);
        for (uint32_t i = 0; i < poseTree.numJoints; i++) {
            auto& node = poseTree[i];
            glm::vec3 v = glm::eulerAngles(currentPose.q(i));
            ImGui::SliderFloat3(node.name.c_str(), (float*)&v, -M_PI, M_PI);
            currentPose.q(i) = glm::rotate(glm::rotate(glm::rotate(
                    glm::identity<glm::quat>(), v.x, {1, 0, 0}), v.y, {0, 1, 0}), v.z, {0, 0, 1});
        }
        ImGui::End();

        phongRenderer.renderImGui();
    }

    void release() override {
    }

private:
    PoseAnimation poseAnim;
    glmx::pose currentPose;

    PoseTree poseTree;
    PoseRenderBody poseRenderBody;

    int frameIdx = 0;
    bool isPlaying = true;

    Ref<PhongMaterial> groundMat;
    Ref<Mesh> groundMesh;
};

int main(int argc, char** argv) {
    MyApp app;
    app.load();
    app.startMainLoop();
    app.release();

    return 0;
}
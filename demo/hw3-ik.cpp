//
// Created by lasagnaphil on 19. 9. 23..
//

#include <InputManager.h>
#include "App.h"
#include "PhongRenderer.h"
#include "DebugRenderer.h"
#include "FlyCamera.h"
#include "Pose.h"
#include "PoseRenderBody.h"
#include "PoseKinematics.h"

#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>

#include <glmx/euler.h>

class MyApp : public App {
public:
    struct IKJacobianSettings {
    };
    struct IKGradientSettings {
        float alpha_targetPos;
        float alpha_targetRot;
        float alpha_poseDiff;
    };

    struct IKSettings {
        enum Type {
            Jacobian, Gradient
        };

        Type type;
        union {
            IKJacobianSettings jacobian;
            IKGradientSettings gradient;
        };

        static IKSettings makeGradient() {
            IKSettings settings;
            settings.type = Gradient;
            settings.gradient.alpha_targetPos = 1.0f;
            settings.gradient.alpha_targetRot = 0.0f;
            settings.gradient.alpha_poseDiff = 0.0f;
            return settings;
        }

        static IKSettings makeJacobian() {
            IKSettings settings;
            settings.type = Jacobian;
            return settings;
        }
    };

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
        MotionClipData::loadFromFile("resources/cmu_07_02_1.bvh", tmpBvh, 0.01f);
        poseTree = tmpBvh.poseTree;

        // Create empty pose
        currentPose = Pose::empty(poseTree.numJoints);
        currentPose.v.y = 1.05f;

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

        // Load IK Example
        ikSettings = IKSettings::makeGradient();
        jointEnabled.resize(currentPose.size(), false);
        jointStiffness.resize(currentPose.size(), 1.0f);
        loadHandIKExample(false);
    }

    void loadHandIKExample(bool addStiffness) {
        currentPose = Pose(currentPose.v, std::vector<glm::quat>(currentPose.size()));
        uint32_t leftHandIdx = poseTree.findIdx("LeftHandIndex1");
        uint32_t spine1Idx = poseTree.findIdx("Spine1");
        rootEnabled = false;
        std::fill_n(jointEnabled.begin(), currentPose.size(), false);
        jointEnabled[leftHandIdx-1] = true;
        jointEnabled[leftHandIdx-2] = true;
        jointEnabled[leftHandIdx-3] = true;
        jointEnabled[leftHandIdx-4] = true;
        jointEnabled[leftHandIdx-5] = true;
        jointEnabled[spine1Idx] = true;
        jointEnabled[spine1Idx-1] = true;
        jointEnabled[spine1Idx-2] = true;
        endEffectorJointIdx = leftHandIdx;

        std::fill_n(jointStiffness.begin(), currentPose.size(), 1.0f);
        if (addStiffness) {
            jointStiffness[spine1Idx] = 128.0f;
            jointStiffness[spine1Idx - 1] = 64.0f;
            jointStiffness[spine1Idx - 2] = 32.0f;
            jointStiffness[leftHandIdx - 1] = 16.0f;
            jointStiffness[leftHandIdx - 2] = 8.0f;
            jointStiffness[leftHandIdx - 3] = 4.0f;
            jointStiffness[leftHandIdx - 4] = 2.0f;
            jointStiffness[leftHandIdx - 5] = 1.0f;
        }

        ikTargetPlane = {{0.f, 1.f, 0.4f}, {0.f, 0.f, 1.f}};
    }

    void loadFeetIKExample(bool addStiffness) {
        currentPose = Pose(currentPose.v, std::vector<glm::quat>(currentPose.size()));
        uint32_t leftToeBaseIdx = poseTree.findIdx("LeftToeBase");
        rootEnabled = false;
        std::fill_n(jointEnabled.begin(), currentPose.size(), false);
        jointEnabled[leftToeBaseIdx - 1] = true;
        jointEnabled[leftToeBaseIdx - 2] = true;
        jointEnabled[leftToeBaseIdx - 3] = true;
        jointEnabled[leftToeBaseIdx - 4] = true;
        endEffectorJointIdx = leftToeBaseIdx;

        std::fill_n(jointStiffness.begin(), currentPose.size(), 1.0f);
        if (addStiffness) {
            jointStiffness[leftToeBaseIdx - 1] = 8.0f;
            jointStiffness[leftToeBaseIdx - 2] = 4.0f;
            jointStiffness[leftToeBaseIdx - 3] = 2.0f;
            jointStiffness[leftToeBaseIdx - 4] = 1.0f;
        }

        ikTargetPlane = {{0.f, 0.f, 0.f}, {0.f, 1.f, 0.f}};
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
        if (inputMgr->isMousePressed(SDL_BUTTON_LEFT)) {
            glm::vec2 mPos = inputMgr->getMousePos();
            Ray ray = camera->screenPointToRay(mPos);
            uint32_t leftHandIdx = poseTree.findIdx("LeftHandIndex1");
            uint32_t spine1Idx = poseTree.findIdx("Spine1");
            std::vector<uint32_t> relevantIndices;
            relevantIndices.reserve(currentPose.size());
            for (int i = 0; i < currentPose.size(); i++) {
                if (jointEnabled[i]) {
                    relevantIndices.push_back(i);
                }
            }
            if (ray.intersectWithPlane(ikTargetPlane, ikTarget)) {
                if (ikSettings.type == IKSettings::Jacobian) {
                    solveIKSimple(poseTree, currentPose, endEffectorJointIdx,
                            relevantIndices,
                            jointStiffness,
                            ikTarget);
                }
                else if (ikSettings.type == IKSettings::Gradient) {
                    std::vector<IKProblem::JointLimit> jointLimits(relevantIndices.size());
                    for (int c = 0; c < relevantIndices.size(); c++) {
                        jointLimits[c].enabled = false;
                        jointLimits[c].minX = -M_PI/2;
                        jointLimits[c].maxX = M_PI/2;
                        jointLimits[c].minY = -M_PI/2;
                        jointLimits[c].maxY = M_PI/2;
                        jointLimits[c].minZ = -M_PI/2;
                        jointLimits[c].maxZ = M_PI/2;
                    }
                    IKProblem ik = {};
                    ik.targetPos = ikTarget;
                    ik.targetRot = currentPose.q[leftHandIdx];
                    ik.targetIdx = endEffectorJointIdx;
                    ik.relevantJoints = relevantIndices;
                    ik.jointLimits = jointLimits;
                    ik.jointStiffness = jointStiffness;
                    ik.alpha_targetPos = ikSettings.gradient.alpha_targetPos;
                    ik.alpha_targetRot = ikSettings.gradient.alpha_targetRot;
                    ik.alpha_poseDiff = ikSettings.gradient.alpha_poseDiff;

                    solveIK(poseTree, currentPose, ik);
                }
                handPos = calcFK(poseTree, currentPose, endEffectorJointIdx).v;
            }
        }
    }

    void render() override {
        phongRenderer.queueRender({groundMesh, groundMat, rootTransform->getWorldTransform()});
        renderMotionClip(phongRenderer, imRenderer, currentPose, poseTree, poseRenderBody);

        phongRenderer.render();

        glm::mat4 rootTransMat = glm::translate(currentPose.v) * glm::mat4_cast(currentPose.q[0]);
        glmx::transform endEffectorTrans = calcFK(poseTree, currentPose, endEffectorJointIdx);
        glm::mat4 endEffectorTransMat = glm::translate(endEffectorTrans.v) * glm::mat4_cast(endEffectorTrans.q);

        imRenderer.drawAxisTriad(rootTransMat, 0.05f, 0.5f, false);
        imRenderer.drawAxisTriad(endEffectorTransMat, 0.05f, 0.5f, false);
        imRenderer.drawSphere(handPos, colors::Red, 0.1f, true);
        imRenderer.drawSphere(ikTarget, colors::Green, 0.1f, true);
        if (ikTargetPlane.dir.y == 0.0) {
            imRenderer.drawPlane(ikTargetPlane.pos, colors::Blue, ikTargetPlane.dir, colors::Blue, 1.0f, 0.1f, true);
        }
        imRenderer.render();

        ImGui::SetNextWindowPos(ImVec2(60, 150), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(500, 900), ImGuiCond_FirstUseEver);

        ImGui::Begin("IK Settings");
        if (ImGui::Button("Hand IK Example")) {
            loadHandIKExample(false);
        }
        ImGui::SameLine();
        if (ImGui::Button("Hand IK Example (with stiffness)")) {
            loadHandIKExample(true);
        }
        if (ImGui::Button("Feet IK Example")) {
            loadFeetIKExample(false);
        }
        ImGui::SameLine();
        if (ImGui::Button("Feet IK Example (with stiffness)")) {
            loadFeetIKExample(true);
        }

        const char* items[] = {"Jacobian", "Gradient"};
        ImGui::Combo("IK Method", (int*) &ikSettings.type, items, IM_ARRAYSIZE(items));
        if (ikSettings.type == IKSettings::Jacobian) {
        }
        else if (ikSettings.type == IKSettings::Gradient) {
        }
        if (ImGui::Button("Reset")) {
            currentPose = Pose(currentPose.v, std::vector<glm::quat>(currentPose.size()));
        }

        static int placeholder = 0;
        ImGui::SliderFloat3("Hip Position", (float*) &currentPose.v, -1.0f, 1.0f);
        ImGui::Text(" Stiffness  Free EE                   Rotation");
        for (uint32_t i = 0; i < poseTree.numJoints; i++) {
            char buff[100];
            snprintf(buff, sizeof(buff), "##Joint %d Stiffness", i);
            ImGui::PushItemWidth(ImGui::GetFontSize() * 6);
            ImGui::InputFloat(buff, (float*)&jointStiffness[i]);
            ImGui::PopItemWidth();

            ImGui::SameLine();
            bool b = jointEnabled[i];
            snprintf(buff, sizeof(buff), "##Joint %d Free", i);
            ImGui::Checkbox(buff, &b);
            jointEnabled[i] = b;

            ImGui::SameLine();
            snprintf(buff, sizeof(buff), "##Joint %d EE", i);
            ImGui::RadioButton(buff, &endEffectorJointIdx, i);

            ImGui::SameLine();
            auto& node = poseTree[i];
            glm::vec3 v = glmx::quatToEuler(currentPose.q[i], EulOrdXYZs);
            ImGui::PushItemWidth(ImGui::GetFontSize() * 24);
            ImGui::SliderFloat3(node.name.c_str(), (float*)&v, -M_PI/2, M_PI/2);
            ImGui::PopItemWidth();
            currentPose.q[i] = glmx::eulerToQuat(v);
        }
        ImGui::End();

        phongRenderer.renderImGui();
    }

    void release() override {
    }

private:
    Pose currentPose;

    PoseTree poseTree;
    PoseRenderBody poseRenderBody;

    Ref<Material> groundMat;
    Ref<Mesh> groundMesh;

    glm::vec3 handPos;
    glm::vec3 ikTarget = {0.f, 0.f, 0.f};
    Ray ikTargetPlane = {{0.f, 1.f, 0.4f}, {0.f, 0.f, 1.f}};
    IKSettings ikSettings;

    bool rootEnabled;
    std::vector<bool> jointEnabled;
    int endEffectorJointIdx;
    std::vector<float> jointStiffness;
};

int main(int argc, char** argv) {
    MyApp app;
    app.start();

    return 0;
}
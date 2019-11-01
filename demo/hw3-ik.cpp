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
    struct IKSettings {
        enum Type {
            Jacobian, Gradient
        };

        Type type = Gradient;
        float alpha_targetPos = 1.0f;
        float alpha_targetRot = 1.0f;
        float alpha_poseDiff = 0.1f;
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
            std::vector<uint32_t> relevantIndices = {
                    leftHandIdx-1, leftHandIdx-2, leftHandIdx-3, leftHandIdx-4, leftHandIdx-5,
                    spine1Idx, spine1Idx-1, spine1Idx-2
            };
            if (ray.intersectWithPlane(ikTargetPlane, ikTarget)) {
                if (ikSettings.type == IKSettings::Jacobian) {
                    solveIKSimple(poseTree, currentPose, leftHandIdx, relevantIndices, ikTarget);
                }
                else if (ikSettings.type == IKSettings::Gradient) {
                    std::vector<IKProblem::JointLimit> jointLimits(relevantIndices.size());
                    for (int c = 0; c < relevantIndices.size(); c++) {
                        jointLimits[c].minX = -M_PI/2;
                        jointLimits[c].maxX = M_PI/2;
                        jointLimits[c].minY = -M_PI/2;
                        jointLimits[c].maxY = M_PI/2;
                        jointLimits[c].minZ = -M_PI/2;
                        jointLimits[c].maxZ = M_PI/2;
                    }
                    IKProblem ik;
                    ik.targetPos = ikTarget;
                    ik.targetRot = currentPose.q[leftHandIdx];
                    ik.targetIdx = leftHandIdx;
                    ik.relevantJoints = relevantIndices;
                    ik.jointLimits = jointLimits;
                    ik.alpha_targetPos = ikSettings.alpha_targetPos;
                    ik.alpha_targetRot = ikSettings.alpha_targetRot;
                    ik.alpha_poseDiff = ikSettings.alpha_poseDiff;

                    solveIK(poseTree, currentPose, ik);
                }
                handPos = calcFK(poseTree, currentPose, leftHandIdx).v;
            }
        }
    }

    void render() override {
        phongRenderer.queueRender({groundMesh, groundMat, rootTransform->getWorldTransform()});
        renderMotionClip(phongRenderer, currentPose, poseTree, poseRenderBody);

        phongRenderer.render();

        glm::mat4 rootTrans = glm::mat4_cast(currentPose.q[0]) * glm::translate(currentPose.v);
        imRenderer.drawAxisTriad(rootTrans, 0.1f, 1.0f, false);
        imRenderer.drawSphere(handPos, colors::Red, 0.1f, true);
        imRenderer.drawSphere(ikTarget, colors::Green, 0.1f, true);
        imRenderer.drawPlane(ikTargetPlane.pos, colors::Blue, ikTargetPlane.dir, colors::Blue, 1.0f, 0.1f, true);
        imRenderer.render();

        ImGui::SetNextWindowPos(ImVec2(60, 150), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(500, 900), ImGuiCond_FirstUseEver);
        ImGui::Begin("Human Control");
        if (ImGui::Button("Reset")) {
            currentPose = Pose(currentPose.v, std::vector<glm::quat>(currentPose.size()));
        }
        ImGui::InputFloat3("Root Position", (float*) &currentPose.v);
        for (uint32_t i = 0; i < poseTree.numJoints; i++) {
            auto& node = poseTree[i];
            glm::vec3 v = glmx::quatToEuler(currentPose.q[i], EulOrdXYZs);
            ImGui::InputFloat3(node.name.c_str(), (float*)&v);
            currentPose.q[i] = glmx::eulerToQuat(v);
        }
        ImGui::End();

        ImGui::Begin("IK Settings");
        const char* items[] = {"Jacobian", "Gradient"};
        ImGui::Combo("combo", (int*) &ikSettings.type, items, IM_ARRAYSIZE(items));
        if (ikSettings.type == IKSettings::Gradient) {
            ImGui::InputFloat("Target importance (pos)", &ikSettings.alpha_targetPos);
            ImGui::InputFloat("Target importance (rot)", &ikSettings.alpha_targetRot);
            ImGui::InputFloat("Pose diff importance", &ikSettings.alpha_poseDiff);
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
};

int main(int argc, char** argv) {
    MyApp app;
    app.start();

    return 0;
}
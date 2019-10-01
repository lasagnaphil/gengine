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

#include "Eigen/Dense"
#include "Eigen/Householder"

#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glmx/transform.h>
#include <glmx/eigen.h>
#include <glmx/euler.h>

glmx::transform calcFK(const PoseTree& poseTree, const Pose& pose, uint32_t mIdx) {
    uint32_t i = mIdx;
    if (poseTree[i].isEndSite) {
        i = poseTree[i].parent;
    }

    glmx::transform t(glm::vec3(0.0f), glm::identity<glm::quat>());

    while (true) {
        auto& node = poseTree[i];
        if (poseTree[i].isEndSite) {
            t = glmx::transform(node.offset) * t;
        }
        else {
            t = glmx::transform(node.offset, pose.q[i]) * t;
        }
        if (i == 0) {
            t = glmx::transform(pose.v) * t;
            break;
        }
        else {
            i = node.parent;
        }
    }
    return t;
}

Eigen::MatrixXf calcEulerJacobian(
        const PoseTree& poseTree, Pose& pose, uint32_t mIdx, nonstd::span<uint32_t> relevantJoints) {

    Eigen::MatrixXf J(3, 3*relevantJoints.size());

    glmx::transform mT = calcFK(poseTree, pose, mIdx);
    int c = 0;
    for (uint32_t i : relevantJoints) {
        glmx::transform axisT = calcFK(poseTree, pose, i);
        glm::vec3 v = mT.v - axisT.v;

        glm::vec3 wx = axisT.q * glm::vec3 {1, 0, 0};
        J.col(3*c) = GLMToEigen(glm::cross(wx, v));
        glm::vec3 wy = axisT.q * glm::vec3 {0, 1, 0};
        J.col(3*c+1) = GLMToEigen(glm::cross(wy, v));
        glm::vec3 wz = axisT.q * glm::vec3 {0, 0, 1};
        J.col(3*c+2) = GLMToEigen(glm::cross(wz, v));

        c++;
    }

    return J;
}

void solveIK(const PoseTree& poseTree, Pose& pose, uint32_t mIdx, glm::vec3 mPos) {
    PoseEuler poseEuler = toEuler(pose, EulOrdXYZs);
    uint32_t relevantJoints[] = {
            mIdx, mIdx-1, mIdx-2, mIdx-3, mIdx-4, mIdx-5
    };
    Eigen::MatrixXf J = calcEulerJacobian(poseTree, pose, mIdx, relevantJoints);
    Eigen::Vector3f dp = GLMToEigen(mPos - calcFK(poseTree, pose, mIdx).v);
    Eigen::VectorXf dq = J.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(dp);
    // Eigen::VectorXf dq = (J.transpose() * J).ldlt().solve(J.transpose() * dp);
    // Eigen::VectorXf dq = J.transpose() * dp;
    int c = 0;
    for (uint32_t i : relevantJoints) {
        assert(i < poseEuler.eulerAngles.size());
        poseEuler.eulerAngles[i].x += dq[3*c];
        poseEuler.eulerAngles[i].y += dq[3*c+1];
        poseEuler.eulerAngles[i].z += dq[3*c+2];
        c++;
    }
    pose = toQuat(poseEuler);
}

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
            uint32_t leftHandIdx = poseTree["LeftHandIndex1"]->childJoints[0];
            if (ray.intersectWithPlane(ikTargetPlane, ikTarget)) {
                handPos = calcFK(poseTree, currentPose, leftHandIdx).v;
                solveIK(poseTree, currentPose, leftHandIdx, ikTarget);
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

        for (uint32_t i = 0; i < poseTree.numJoints; i++) {
            auto& node = poseTree[i];
            glm::vec4 v = glmx::quatToEuler(currentPose.q[i], EulOrdXYZs);
            ImGui::InputFloat3(node.name.c_str(), (float*)&v);
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
    Ray ikTargetPlane = {{0.f, 1.f, 0.5f}, {0.f, 0.f, 1.f}};
};

int main(int argc, char** argv) {
    MyApp app;
    app.start();

    return 0;
}
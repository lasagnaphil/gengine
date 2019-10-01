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

Eigen::Vector3f GLMToEigen(const glm::vec3& v) {
    return Eigen::Vector3f(v.x, v.y, v.z);
}

namespace glmx {
    struct transform {
        glm::vec3 v;
        glm::quat q;

        transform() = default;
        transform(glm::vec3 v) : v(v), q(glm::identity<glm::quat>()) {}
        transform(glm::quat q) : v(glm::vec3()), q(q) {}
        transform(glm::vec3 v, glm::quat q) : v(v), q(q) {}
    };

    struct transform_disp {
        glm::vec3 v;
        glm::vec3 u;

        transform_disp() = default;
        transform_disp(glm::vec3 v, glm::vec3 u) : v(v), u(u) {}
    };

    inline glm::mat4 mat4_cast(const transform& t) {
        return translate(mat4_cast(t.q), t.v);
    }

    inline transform operator*(const transform& t1, const transform& t2) {
        return {t1.q * t2.v + t1.v, t1.q * t2.q};
    }

    inline transform operator/(const transform& t1, const transform& t2) {
        return {conjugate(t2.q) * (t1.v - t2.v), glm::conjugate(t2.q) * t1.q};
    }

    inline transform_disp operator+(const transform_disp& d1, const transform_disp& d2) {
        return {d1.v + d2.v, d1.u + d2.u};
    }

    inline transform_disp operator-(const transform_disp& d1, const transform_disp& d2) {
        return {d1.v - d2.v, d1.u - d2.u};
    }

    inline transform_disp operator*(float k, const transform_disp& d) {
        return {k * d.v, k * d.u};
    }

    inline transform_disp operator/(float k, const transform_disp& d) {
        return {d.v / k, d.u / k};
    }

    inline transform exp(const transform_disp& d) {
        return {d.v, glm::angleAxis(glm::length(d.u), glm::normalize(d.u))};
    }

    inline transform_disp log(const transform& t) {
        return {t.v, glm::angle(t.q) * glm::axis(t.q)};
    }

}

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

/*
Eigen::MatrixXf calcJacobianApprox(
        const PoseTree& poseTree, Pose pose, uint32_t mIdx) {

    constexpr float dv = 1e-3;

    Eigen::MatrixXf J(3, pose.size());

    // TODO: Not finished
    Pose newPose = pose;
    for (int i = 0; i < poseTree.numJoints; i++) {
        glm::vec3 p = calcFK(poseTree, pose, mIdx);
        for (int d = 0; d < 3; d++) {
            newPose.q[i][d] += dv;
            float p_d = calcFK(poseTree, newPose, mIdx)[d];
            J(d, i) = (p_d - p[d]) / dv;
            newPose.q[i][d] = pose.q[i][d];
        }
    }

    return J;
}
 */

Eigen::MatrixXf calcEulerJacobian(
        const PoseTree& poseTree, Pose& pose, uint32_t mIdx) {

    Eigen::MatrixXf J(3, 3*pose.size());

    glmx::transform mT = calcFK(poseTree, pose, mIdx);
    for (int i = 0; i < pose.size(); i++) {
        glmx::transform axisT = calcFK(poseTree, pose, i);
        glm::vec3 v = axisT.v - mT.v;

        glm::vec3 wx = axisT.q * glm::vec3 {1, 0, 0};
        J.col(3*i) = GLMToEigen(glm::cross(wx, v));
        glm::vec3 wy = axisT.q * glm::vec3 {0, 1, 0};
        J.col(3*i+1) = GLMToEigen(glm::cross(wy, v));
        glm::vec3 wz = axisT.q * glm::vec3 {0, 0, 1};
        J.col(3*i+2) = GLMToEigen(glm::cross(wz, v));
    }

    return J;
}

void solveIK(const PoseTree& poseTree, Pose& pose, uint32_t mIdx, glm::vec3 mPos) {
    PoseEuler poseEuler = toEuler(pose);
    Eigen::MatrixXf J = calcEulerJacobian(poseTree, pose, mIdx);
    Eigen::Vector3f dp = GLMToEigen(mPos - calcFK(poseTree, pose, mIdx).v);
    Eigen::VectorXf dq = J.colPivHouseholderQr().solve(dp);
    for (int i = 0; i < pose.size(); i++) {
        poseEuler.eulerAngles[i] += glm::vec3(dq[3*i], dq[3*i+1], dq[3*i+2]);
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
    }

    void render() override {
        phongRenderer.queueRender({groundMesh, groundMat, rootTransform->getWorldTransform()});
        renderMotionClip(phongRenderer, currentPose, poseTree, poseRenderBody);

        phongRenderer.render();

        glm::mat4 rootTrans = glm::mat4_cast(currentPose.q[0]) * glm::translate(currentPose.v);
        imRenderer.drawAxisTriad(rootTrans, 0.1f, 1.0f, false);
        // imRenderer.drawBox(glm::vec3 {1.0f, 1.0f, 1.0f}, colors::Blue, 1.0f, 1.0f, 1.0f, false);
        imRenderer.render();

        ImGui::SetNextWindowPos(ImVec2(60, 150), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(500, 900), ImGuiCond_FirstUseEver);
        ImGui::Begin("Human Control");

        for (uint32_t i = 0; i < poseTree.numJoints; i++) {
            auto& node = poseTree[i];
            glm::vec3 v = glm::eulerAngles(currentPose.q[i]);
            ImGui::SliderFloat3(node.name.c_str(), (float*)&v, -M_PIf32, M_PIf32);
            currentPose.q[i] = glm::rotate(glm::rotate(glm::rotate(
                    glm::identity<glm::quat>(), v.x, {1, 0, 0}), v.y, {0, 1, 0}), v.z, {0, 0, 1});
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
};

int main(int argc, char** argv) {
    MyApp app;
    app.start();

    return 0;
}
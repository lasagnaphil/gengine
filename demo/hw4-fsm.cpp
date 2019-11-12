//
// Created by lasagnaphil on 19. 11. 6..
//

#include <InputManager.h>
#include "App.h"
#include "PhongRenderer.h"
#include "DebugRenderer.h"
#include "FlyCamera.h"
#include "Pose.h"
#include "PoseRenderBody.h"
#include "PoseKinematics.h"
#include "AnimStateMachine.h"

#include <glmx/euler.h>

class MyApp : public App {
public:
    MyApp() : App(false) {}

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
        MotionClipData tmpBvh = MotionClipData::loadFromFile("resources/cmu_07_02_1.bvh", 0.01f);
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

        auto walkBVH = MotionClipData::loadFromFile("resources/motion/cmu/16_15_walk.bvh", 0.01f);
        auto idleAnim = animFSM.addAnimation("walk", nonstd::span<Pose>(&walkBVH.poseStates[0], 1));
        auto walkAnim = animFSM.addAnimation("walk", walkBVH.poseStates);

        auto jumpBVH = MotionClipData::loadFromFile("resources/motion/cmu/16_03_high jump.bvh", 0.01f);
        auto jumpAnim = animFSM.addAnimation("jump", jumpBVH.poseStates);

        auto forwardJumpBVH = MotionClipData::loadFromFile("resources/motion/cmu/16_05_forward jump.bvh", 0.01f);
        auto forwardJumpAnim = animFSM.addAnimation("forward_jump", forwardJumpBVH.poseStates);

        auto idleState = animFSM.addState("idle", idleAnim);
        auto walkState = animFSM.addState("walk", walkAnim);
        auto jumpState = animFSM.addState("jump", jumpAnim);
        auto forwardJumpState = animFSM.addState("forward_jump", forwardJumpAnim);

        animFSM.addParam("is_walking", false);
        animFSM.addParam("jump");
        animFSM.addParam("forward_jump");

        auto startWalkingTrans = animFSM.addTransition("start_walking", idleState, walkState, 1.0f);
        animFSM.addCondition(startWalkingTrans, "is_walking", true);
        auto stopWalkingTrans = animFSM.addTransition("stop_walking", walkState, idleState, 1.0f);
        animFSM.addCondition(stopWalkingTrans, "is_walking", false);
        auto jumpTrans = animFSM.addTransition("jumping", idleState, jumpState, 1.0f);
        animFSM.addTrigger(jumpTrans, "jump");
        auto forwardJumpTrans = animFSM.addTransition("forward_jumping", walkState, forwardJumpState, 1.0f);
        animFSM.addTrigger(forwardJumpTrans, "forward_jump");

        animFSM.setCurrentState(walkState);
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
        }
        animFSM.update(dt);
        currentPose = animFSM.getCurrentPose();
    }

    void render() override {
        phongRenderer.queueRender({groundMesh, groundMat, rootTransform->getWorldTransform()});
        renderMotionClip(phongRenderer, imRenderer, currentPose, poseTree, poseRenderBody);

        phongRenderer.render();

        glm::mat4 rootTransMat = glm::translate(currentPose.v) * glm::mat4_cast(currentPose.q[0]);

        imRenderer.drawAxisTriad(rootTransMat, 0.05f, 0.5f, false);

        imRenderer.render();

        phongRenderer.renderImGui();
    }

    void release() override {
    }

private:
    Pose currentPose;

    PoseTree poseTree;
    PoseRenderBody poseRenderBody;

    AnimStateMachine animFSM;

    Ref<Material> groundMat;
    Ref<Mesh> groundMesh;
};

int main(int argc, char** argv) {
    MyApp app;
    app.start();

    return 0;
}


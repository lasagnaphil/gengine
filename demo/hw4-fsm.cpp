//
// Created by lasagnaphil on 19. 11. 6..
//

#include <InputManager.h>
#include "App.h"
#include "PhongRenderer.h"
#include "DebugRenderer.h"
#include "FlyCamera.h"
#include "glmx/pose.h"
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

        // Create empty pose
        // Material of human
        Ref<Material> bodyMat = Resources::make<Material>();
        bodyMat->ambient = {0.1f, 0.1f, 0.1f, 1.0f};
        bodyMat->specular = {0.7f, 0.7f, 0.7f, 1.0f};
        bodyMat->diffuse = {1.0f, 0.0f, 0.0f, 1.0f};
        bodyMat->shininess = 64.0f;
        bodyMat->texDiffuse = {};
        bodyMat->texSpecular = {};

        auto walkBVH = MotionClipData::loadFromFile("resources/motion/cmu/16_15_walk.bvh", 0.01f);

        poseTree = walkBVH.poseTree;
        currentPose = glmx::pose::empty(poseTree.numJoints);
        currentPose.v.y = 1.05f;

        poseRenderBody = PoseRenderBody::createAsBoxes(poseTree, 0.05f, bodyMat);

        auto runBVH = MotionClipData::loadFromFile("resources/motion/cmu/16_35_run&jog.bvh", 0.01f);
        auto jumpBVH = MotionClipData::loadFromFile("resources/motion/cmu/16_03_high jump.bvh", 0.01f);
        auto forwardJumpBVH = MotionClipData::loadFromFile("resources/motion/cmu/16_05_forward jump.bvh", 0.01f);

        auto idleAnim = animFSM.addAnimation("idle", nonstd::span<glmx::pose>(&walkBVH.poseStates[0], 1));
        auto walkAnim = animFSM.addAnimation("walk", walkBVH.poseStates);
        auto runAnim = animFSM.addAnimation("run", runBVH.poseStates);
        auto jumpAnim = animFSM.addAnimation("jump", jumpBVH.poseStates);
        auto forwardJumpAnim = animFSM.addAnimation("forward_jump", forwardJumpBVH.poseStates);

        auto idleState = animFSM.addState("idle", idleAnim);
        animFSM.get(idleState)->loop = true;
        auto walkState = animFSM.addState("walk", walkAnim);
        animFSM.get(walkState)->loop = true;
        auto runState = animFSM.addState("run", runAnim);
        animFSM.get(runState)->loop = true;
        auto jumpState = animFSM.addState("jump", jumpAnim);
        auto forwardJumpState = animFSM.addState("forward_jump", forwardJumpAnim);

        animFSM.addParam("is_walking", false);
        animFSM.addParam("is_running", false);
        animFSM.addParam("jump");
        animFSM.addParam("forward_jump");

        auto startWalkingTrans = animFSM.addTransition("start_walking", idleState, walkState, 0.2f);
        animFSM.addCondition(startWalkingTrans, "is_walking", true);
        auto stopWalkingTrans = animFSM.addTransition("stop_walking", walkState, idleState, 0.2f);
        animFSM.addCondition(stopWalkingTrans, "is_walking", false);
        auto startRunningTrans = animFSM.addTransition("start_running", walkState, runState, 0.2f);
        animFSM.addCondition(startRunningTrans, "is_running", true);
        auto stopRunningTrans = animFSM.addTransition("stop_running", runState, walkState, 0.2f);
        animFSM.addCondition(stopRunningTrans, "is_running", false);

        auto jumpTrans = animFSM.addTransition("jumping", idleState, jumpState, 1.0f);
        // animFSM.addTrigger(jumpTrans, "jump");
        auto forwardJumpTrans = animFSM.addTransition("forward_jumping", walkState, forwardJumpState, 1.0f);
        // animFSM.addTrigger(forwardJumpTrans, "forward_jump");

        animFSM.setCurrentState(idleState);
    }

    void processInput(SDL_Event &event) override {
    }

    void update(float dt) override {
        static float time = 0.0f;
        time += dt;
        auto inputMgr = InputManager::get();
        if (inputMgr->isMousePressed(SDL_BUTTON_LEFT)) {
            glm::vec2 mPos = inputMgr->getMousePos();
            Ray ray = camera->screenPointToRay(mPos);
        }
        if (inputMgr->isKeyPressed(SDL_SCANCODE_UP)) {
            animFSM.setParam("is_walking", true);
            if (inputMgr->isKeyPressed(SDL_SCANCODE_LSHIFT)) {
                animFSM.setParam("is_running", true);
            }
            else {
                animFSM.setParam("is_running", false);
            }
        }
        else {
            animFSM.setParam("is_walking", false);
            animFSM.setParam("is_running", false);
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
        renderImGui();
    }

    void renderImGui() {
        animFSM.renderImGui(poseTree);
    }

    void release() override {
    }

private:
    glmx::pose currentPose;

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


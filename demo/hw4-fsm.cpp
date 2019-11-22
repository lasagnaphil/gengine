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

        // Create empty pose
        // Material of human
        Ref<PhongMaterial> bodyMat = Resources::make<PhongMaterial>();
        bodyMat->ambient = {0.1f, 0.1f, 0.1f, 1.0f};
        bodyMat->specular = {0.7f, 0.7f, 0.7f, 1.0f};
        bodyMat->diffuse = {1.0f, 0.0f, 0.0f, 1.0f};
        bodyMat->shininess = 64.0f;
        bodyMat->texDiffuse = {};
        bodyMat->texSpecular = {};

        Ref<PhongMaterial> debugBodyMat1 = Resources::make<PhongMaterial>();
        debugBodyMat1->ambient = {0.1f, 0.1f, 0.1f, 1.0f};
        debugBodyMat1->specular = {0.7f, 0.7f, 0.7f, 1.0f};
        debugBodyMat1->diffuse = {0.0f, 1.0f, 0.0f, 1.0f};
        debugBodyMat1->shininess = 64.0f;
        debugBodyMat1->texDiffuse = {};
        debugBodyMat1->texSpecular = {};

        Ref<PhongMaterial> debugBodyMat2 = Resources::make<PhongMaterial>();
        debugBodyMat2->ambient = {0.1f, 0.1f, 0.1f, 1.0f};
        debugBodyMat2->specular = {0.7f, 0.7f, 0.7f, 1.0f};
        debugBodyMat2->diffuse = {0.0f, 0.0f, 1.0f, 1.0f};
        debugBodyMat2->shininess = 64.0f;
        debugBodyMat2->texDiffuse = {};
        debugBodyMat2->texSpecular = {};

        auto walkBVH = MotionClipData::loadFromFile("resources/motion/cmu/16_15_walk.bvh", 0.01f);

        poseTree = walkBVH.poseTree;
        currentPose = glmx::pose::empty(poseTree.numJoints);
        currentPose.v.y = 1.05f;

        poseRenderBody = PoseRenderBody::createAsBoxes(poseTree, 0.05f, bodyMat);
        debugPoseRenderBody1 = PoseRenderBody::createAsBoxes(poseTree, 0.05f, debugBodyMat1);
        debugPoseRenderBody2 = PoseRenderBody::createAsBoxes(poseTree, 0.05f, debugBodyMat2);

        auto runBVH = MotionClipData::loadFromFile("resources/motion/cmu/16_35_run&jog.bvh", 0.01f);
        auto jumpBVH = MotionClipData::loadFromFile("resources/motion/cmu/16_03_high jump.bvh", 0.01f);
        auto forwardJumpBVH = MotionClipData::loadFromFile("resources/motion/cmu/16_05_forward jump.bvh", 0.01f);
        auto walkVeerLeftBVH = MotionClipData::loadFromFile("resources/motion/cmu/16_11_walk, veer left.bvh", 0.01f);
        auto walkVeerRightBVH = MotionClipData::loadFromFile("resources/motion/cmu/16_13_walk, veer right.bvh", 0.01f);

        auto idlePoses = std::vector<glmx::pose>(30, jumpBVH.poseStates[0]);
        auto idleAnim = animFSM.addAnimation("idle", nonstd::span<glmx::pose>(idlePoses.data(), idlePoses.size()));
        auto walkAnim = animFSM.addAnimation("walk", walkBVH.poseStates);
        auto walkVeerLeftAnim = animFSM.addAnimation("walk_veer_left", walkVeerLeftBVH.poseStates);
        auto walkVeerRightAnim = animFSM.addAnimation("walk_veer_right", walkVeerRightBVH.poseStates);
        auto runAnim = animFSM.addAnimation("run", runBVH.poseStates);
        auto jumpAnim = animFSM.addAnimation("jump", jumpBVH.poseStates);
        auto forwardJumpAnim = animFSM.addAnimation("forward_jump", forwardJumpBVH.poseStates);

        for (Ref<Animation> anim : {idleAnim, walkAnim, walkVeerLeftAnim, walkVeerRightAnim,
                                    runAnim, jumpAnim, forwardJumpAnim}) {

            animFSM.get(anim)->setStartingRootPos(0.0f, 0.0f);
        }

        auto idleState = animFSM.addState("idle", idleAnim);
        auto walkState = animFSM.addState("walk", walkAnim);
        auto runState = animFSM.addState("run", runAnim);
        auto jumpState = animFSM.addState("jump", jumpAnim);
        auto forwardJumpState = animFSM.addState("forward_jump", forwardJumpAnim);
        auto walkVeerLeftState = animFSM.addState("walk_veer_left", walkVeerLeftAnim);
        auto walkVeerRightState = animFSM.addState("walk_veer_right", walkVeerRightAnim);

        animFSM.addParam("is_walking", false);
        animFSM.addParam("is_walking_veer_left");
        animFSM.addParam("is_walking_veer_right");
        animFSM.addParam("is_running", false);
        animFSM.addParam("jump");
        animFSM.addParam("forward_jump");

        auto repeatIdleTrans = animFSM.addTransition("repeat_idle", idleState, idleState, 0.0f, 0.0f, 0.0f);

        auto repeatWalkingTrans = animFSM.addTransition("repeat_walking", walkState, walkState, 0.1f, 0.1f, 0.1f);

        auto startWalkingTrans = animFSM.addTransition("start_walking", idleState, walkState, 0.2f, 0.2f, 0.2f);
        animFSM.setTransitionCondition(startWalkingTrans, "is_walking", true);

        auto stopWalkingTrans = animFSM.addTransition("stop_walking", walkState, idleState, 0.2f, 0.2f, 0.2f);
        animFSM.setTransitionCondition(stopWalkingTrans, "is_walking", false);

        auto startWalkVeerLeftTrans = animFSM.addTransition("start_walk_veer_left", walkState, walkVeerLeftState,
                0.2f, 0.2f, 0.2f);
        animFSM.setTransitionTrigger(startWalkVeerLeftTrans, "is_walking_veer_left");

        auto stopWalkVeerLeftTrans = animFSM.addTransition("stop_walk_veer_left", walkVeerLeftState, walkState,
                                                            0.2f, 0.2f, 0.2f);

        auto startWalkVeerRightTrans = animFSM.addTransition("start_walk_veer_right", walkState, walkVeerRightState,
                0.2f, 0.2f, 0.2f);
        animFSM.setTransitionTrigger(startWalkVeerRightTrans, "is_walking_veer_right");

        auto stopWalkVeerRightTrans = animFSM.addTransition("stop_walk_veer_right", walkVeerRightState, walkState,
                                                             0.2f, 0.2f, 0.2f);

        auto repeatRunningTrans = animFSM.addTransition("repeat_running", runState, runState, 0.1f, 0.1f, 0.1f);

        auto startRunningTrans = animFSM.addTransition("start_running", walkState, runState, 0.2f, 0.2f, 0.2f);
        animFSM.setTransitionCondition(startRunningTrans, "is_running", true);

        auto stopRunningTrans = animFSM.addTransition("stop_running", runState, walkState, 0.2f, 0.2f, 0.2f);
        animFSM.setTransitionCondition(stopRunningTrans, "is_running", false);

        auto jumpTrans = animFSM.addTransition("jumping", idleState, jumpState, 0.2f, 0.2f, 0.2f);
        animFSM.setTransitionTrigger(jumpTrans, "jump");

        auto stopJumpTrans = animFSM.addTransition("stop_jumping", jumpState, idleState, 0.2f, 0.2f, 0.2f);

        auto forwardJumpTrans = animFSM.addTransition("forward_jumping", walkState, forwardJumpState, 0.2f, 0.2f, 0.2f);
        animFSM.setTransitionTrigger(forwardJumpTrans, "forward_jump");

        auto stopForwardJumpTrans = animFSM.addTransition("stop_forward_jumping", forwardJumpState, walkState,
                0.2f, 0.2f, 0.2f);

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

            if (inputMgr->isKeyPressed(SDL_SCANCODE_LEFT)) {
                animFSM.setTrigger("is_walking_veer_left");
            }
            if (inputMgr->isKeyPressed(SDL_SCANCODE_RIGHT)) {
                animFSM.setTrigger("is_walking_veer_right");
            }
            if (inputMgr->isKeyPressed(SDL_SCANCODE_SPACE)) {
                animFSM.setTrigger("forward_jump");
            }

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
        if (inputMgr->isKeyPressed(SDL_SCANCODE_SPACE)) {
            animFSM.setTrigger("jump");
        }
        animFSM.update(dt);
        currentPose = animFSM.getCurrentPose();
    }

    void render() override {
        phongRenderer.queueRender({groundMesh, groundMat, rootTransform->getWorldTransform()});
        renderMotionClip(phongRenderer, imRenderer, currentPose, poseTree, poseRenderBody);

        /*
        if (animFSM.p1.size() > 0)
            renderMotionClip(phongRenderer, imRenderer, animFSM.p1, poseTree, debugPoseRenderBody1);
        if (animFSM.p2.size() > 0)
            renderMotionClip(phongRenderer, imRenderer, animFSM.p2, poseTree, debugPoseRenderBody2);
        */

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
    PoseRenderBody debugPoseRenderBody1, debugPoseRenderBody2;

    AnimStateMachine animFSM;

    Ref<PhongMaterial> groundMat;
    Ref<Mesh> groundMesh;
};

int main(int argc, char** argv) {
    MyApp app;
    app.start();

    return 0;
}


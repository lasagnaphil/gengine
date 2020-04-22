#include "InputManager.h"
#include "App.h"
#include "PBRenderer.h"
#include "FlyCamera.h"
#include "physx/PhysicsWorld.h"
#include "physx/PosePhysicsBody.h"
#include "physx/PhysicsBody.h"
#include "physx/PhysXDebugRenderer.h"
#include "PoseRenderBody.h"
#include "anim/BVHData.h"
#include "anim/PoseFK.h"
#include "anim/PoseIK.h"
#include "anim/AnimStateMachine.h"

PxDefaultAllocator gAllocator = {};
PxDefaultErrorCallback gErrorCallback = {};

struct PhysicsObject {
    PhysicsBody body;
    Ref<Mesh> mesh;
    Ref<PBRMaterial> material;
};

class MyApp : public App {
public:
    MyApp() : App(false, AppSettings::defaultPBR()) {}

    void loadResources() override {
        FlyCamera* camera = dynamic_cast<FlyCamera*>(this->camera.get());
        camera->movementSpeed = 1.0f;
        Ref<Transform> cameraTransform = camera->transform;
        cameraTransform->setPosition({0.0f, 0.0f, 1.0f});

        pbRenderer.dirLightProjVolume = {
                {-10.f, -10.f, 0.f}, {10.f, 10.f, 1000.f}
        };
        pbRenderer.shadowFramebufferSize = {2048, 2048};

        pbRenderer.dirLight.enabled = true;
        pbRenderer.dirLight.direction = glm::normalize(glm::vec3 {2.0f, -3.0f, -2.0f});
        pbRenderer.dirLight.color = {1.f, 1.f, 1.f};

        pbRenderer.pointLights[0].enabled = true;
        pbRenderer.pointLights[0].position = {-2.0f, 3.0f, -5.0f};
        pbRenderer.pointLights[0].color = {30.f, 30.f, 30.f};

        pbRenderer.pointLights[1].enabled = true;
        pbRenderer.pointLights[1].position = {2.0f, 3.0f, -5.0f};
        pbRenderer.pointLights[1].color = {30.f, 30.f, 30.f};

        /*
        pbRenderer.pointLights[2].enabled = true;
        pbRenderer.pointLights[2].position = {-10.0f, 10.0f + 17.5f, 10.0f};
        pbRenderer.pointLights[2].color = {30.f, 30.f, 30.f};

        pbRenderer.pointLights[3].enabled = true;
        pbRenderer.pointLights[3].position = {10.0f, 10.0f + 17.5f, 10.0f};
        pbRenderer.pointLights[3].color = {30.f, 30.f, 30.f};
         */

        // Initialize the PhysX Engine
        world.init(1, false);

        pxDebugRenderer.init(world);
        pxDebugRenderer.setCamera(camera);

        // Prepare the ground

        groundMat = PBRMaterial::quick(
                "resources/textures/mossy-ground1-albedo.png",
                "resources/textures/mossy-ground1-metal.png",
                "resources/textures/mossy-ground1-roughness.png",
                "resources/textures/mossy-ground1-ao.png");

        groundMesh = Mesh::makePlane(100.0f, 100.0f);

        auto defaultAO = Texture::fromSingleColor({1.0f, 0.0f, 0.0f});
        auto defaultMetallic = Texture::fromSingleColor({0.5f, 0.0f, 0.0f});
        auto defaultRoughness = Texture::fromSingleColor({0.5f, 0.0f, 0.0f});

        // Prepare the box
        if (enableBox) {
            box.mesh = Mesh::fromOBJFile("resources/box.obj", true, false);
            box.material = Resources::make<PBRMaterial>();
            box.material->texAlbedo = Texture::fromSingleColor(glm::vec3(0.4f));
            box.material->texAO = defaultAO;
            box.material->texMetallic = defaultMetallic;
            box.material->texRoughness = defaultRoughness;
            box.mesh->indices.clear(); // We don't need the index buffer
            box.body = PhysicsBody::ourBox(world, boxSize.x, boxSize.y, boxSize.z, boxThickness,
                    world.physics->createMaterial(0.5f, 0.5f, 0.6f));
            box.body.setKinematic(true);
        }
        if (enableSpheres) {
            // Prepare the spheres
            glm::vec3 colorList[] = {
                    colors::Red, colors::Blue, colors::Green, colors::Black, colors::Yellow, colors::Cyan, colors::Pink,
                    colors::White, colors::Magenta, colors::Gold
            };
            std::vector<Ref<PBRMaterial>> sphereMats(sphereColorsCount);
            for (int i = 0; i < sphereColorsCount; i++) {
                sphereMats[i] = Resources::make<PBRMaterial>();
                sphereMats[i]->texAlbedo = Texture::fromSingleColor(colorList[i]);
                sphereMats[i]->texAO = defaultAO;
                sphereMats[i]->texMetallic = defaultMetallic;
                sphereMats[i]->texRoughness = defaultRoughness;
            }

            Ref<Mesh> sphereMesh = Mesh::makeSphere(sphereRadius);

            spheres.resize(sphereCount);
            for (int i = 0; i < sphereCount; i++) {
                spheres[i].body = PhysicsBody::sphere(world, world.physics->createMaterial(0.5f, 0.5f, 0.6f), 0.5f,
                        {}, sphereRadius);
                spheres[i].mesh = sphereMesh;
                spheres[i].material = sphereMats[i % sphereColorsCount];
            }
        }

        if (enableObstacle) {
            obstacle.body = PhysicsBody::box(world, world.physics->createMaterial(0.5f, 0.5f, 0.6f), 10.0f,
                    glm::vec3(-5, 0, 0), glm::identity<glm::quat>(), obstacleSize);
            obstacle.body.setKinematic(true);
            obstacle.mesh = Mesh::makeCube(obstacleSize);
            obstacle.material = Resources::make<PBRMaterial>();
            obstacle.material->texAlbedo = Texture::fromSingleColor(colors::White);
            obstacle.material->texAO = defaultAO;
            obstacle.material->texMetallic = defaultMetallic;
            obstacle.material->texRoughness = defaultRoughness;
        }

        // Prepare motion clip
        pickupBVH = BVHData::loadFromFile("resources/pick_up.bvh", 0.01f);
        auto walkBVH = BVHData::loadFromFile("resources/carry.bvh", 0.01f);

        //bvh = walkBVH;

        poseTree = pickupBVH.poseTree;
        currentPose = glmx::pose::empty(poseTree.numJoints);

        auto pickupPoses = pickupBVH.slice(1, pickupBVH.numFrames);
        auto idlePoses = std::vector<glmx::pose>(120, pickupBVH.poseStates[pickupBVH.numFrames - 1]);
        // auto walkPoses = walkBVH.slice(240, walkBVH.numFrames);
        auto walkPoses = walkBVH.slice(1063, 1201);

        auto pickupAnim = animFSM.addAnimation("pickup", pickupPoses, 120);
        auto idleAnim = animFSM.addAnimation("idle", idlePoses, 120);
        auto walkAnim = animFSM.addAnimation("walk", walkPoses, 120);

        for (Ref<Animation> anim : { pickupAnim, idleAnim, walkAnim })
        {
            animFSM.get(anim)->setStartingRootPos(0.0f, 0.0f);
        }

        pickupState = animFSM.addState("pickup", pickupAnim);
        auto idleState = animFSM.addState("idle", idleAnim);
        auto walkState = animFSM.addState("walk", walkAnim);

        animFSM.addParam("is_walking", false);

        auto finishPickupTrans = animFSM.addTransition("finish_pickup", pickupState, idleState, 0.2f, 0.2f, 0.2f);

        auto repeatIdleTrans = animFSM.addTransition("repeat_idle", idleState, idleState, 0.0f, 0.0f, 0.0f);

        auto repeatWalkingTrans = animFSM.addTransition("repeat_walking", walkState, walkState, 0.1f, 0.1f, 0.1f);

        auto startWalkingTrans = animFSM.addTransition("start_walking", idleState, walkState, 0.1f, 0.1f, 0.1f);
        animFSM.setTransitionCondition(startWalkingTrans, "is_walking", true);

        auto stopWalkingTrans = animFSM.addTransition("stop_walking", walkState, idleState, 0.1f, 0.1f, 0.1f);
        animFSM.setTransitionCondition(stopWalkingTrans, "is_walking", false);


        // Prepare the human body

        posePhysicsBodySkel = PosePhysicsBodySkel::fromFile("resources/humanoid.xml", poseTree);
        posePhysicsBody.init(world, poseTree, posePhysicsBodySkel);
        posePhysicsBody.setRoot(glmx::transform(glm::vec3(0.f, 0.9f, 0.f)));


        Ref<PBRMaterial> poseBodyMat = Resources::make<PBRMaterial>();
        poseBodyMat->texAlbedo = Texture::fromSingleColor({0.5f, 0.0f, 0.0f});
        poseBodyMat->texAO = Texture::fromSingleColor({1.0f, 0.0f, 0.0f});
        poseBodyMat->texMetallic =
                Texture::fromSingleColor({0.5f, 0.0f, 0.0f});
        poseBodyMat->texRoughness =
                Texture::fromSingleColor({0.5f, 0.0f, 0.0f});
        poseRenderBodySimple = PoseRenderBodyPBR::createAsBoxes(poseTree, 0.02, poseBodyMat);

        std::vector<Ref<PBRMaterial>> mats(poseTree.numNodes);
        for (int i = 0; i < poseTree.numNodes; i++) {
            mats[i] = Resources::make<PBRMaterial>();
            mats[i]->texAlbedo = Texture::fromSingleColor(colors::WhiteSmoke);
            mats[i]->texAO = Texture::fromSingleColor({1.0f, 0.0f, 0.0f});
            mats[i]->texMetallic = Texture::fromSingleColor({0.8f, 0.0f, 0.0f});
            mats[i]->texRoughness = Texture::fromSingleColor({0.8f, 0.0f, 0.0f});
        }
        poseRenderBody = createFromSkel(poseTree, posePhysicsBodySkel, mats);

        reset();
    }

    void reset() {
        enableRagdoll = false;
        isHoldingBox = false;
        animFSM.setCurrentState(pickupState);
        currentPose = animFSM.getCurrentPose();

        posePhysicsBody.setPose(currentPose, poseTree);

        if (enableBox) {
            box.body.setKinematic(true);
            auto pickupPose = animFSM.getCurrentAnim().getFrame(1.635f);
            auto leftHandTrans = calcFK(poseTree, pickupPose, poseTree.findIdx("LeftHand"));
            auto rightHandTrans = calcFK(poseTree, pickupPose, poseTree.findIdx("RightHand"));
            glmx::transform boxTrans;
            boxTrans.v.x = 0.5f * (leftHandTrans.v.x + rightHandTrans.v.x);
            boxTrans.v.y = 0.0f;
            boxTrans.v.z = pickupPose.v.z - 0.4f;
            float theta = atan2(rightHandTrans.v.z - leftHandTrans.v.z, rightHandTrans.v.x - leftHandTrans.v.x);
            boxTrans.q = glm::rotate(-theta, glm::vec3(0, 1, 0));
            box.body.setTransform(boxTrans);
            box.body.setLinearVelocity({});
            box.body.setAngularVelocity({});
        }

        if (enableSpheres) {
            glm::vec3 boxPos = box.body.getTransform().v;
            for(int x = 0; x < sphereCountWidth; x++) {
                for(int y = 0; y < sphereCountWidth; y++) {
                    for(int z = 0; z < sphereCountWidth; z++) {
                        int i = x * sphereCountWidth * sphereCountWidth + y * sphereCountWidth + z;
                        spheres[i].body.setPosition(
                                glm::vec3(boxPos.x - boxSize.x + 3.0f * sphereRadius + x * 2.0f * (sphereRadius + sphereDist),
                                          boxPos.y + sphereRadius + boxThickness * 2.0f + y * 2.0f * (sphereRadius + sphereDist),
                                          boxPos.z - boxSize.z + 3.0f * sphereRadius + z * 2.0f * (sphereRadius + sphereDist)));
                        spheres[i].body.setLinearVelocity({});
                        spheres[i].body.setAngularVelocity({});
                    }
                }
            }
        }

        if (enableObstacle) {
            obstacle.body.setPosition(glm::vec3(0.0f, obstacleSize.y/2, -5.0f));
        }
    }

    void startRagdoll() {
        float dt = 1.0f / 120.0f;
        enableRagdoll = true;
        posePhysicsBody.setPose(currentPose, poseTree);

        // Set velocity to articulated body (for more realism)
        float t = animFSM.getStateTime();
        glmx::pose beforePose = animFSM.getCurrentAnim().getFrame(t - dt);
        posePhysicsBody.setPoseVelocityFromTwoPoses(beforePose, currentPose, dt);
        box.body.setKinematic(false);

        // To trip even more spectacularily, add some forces and torques
        posePhysicsBody.applyImpulseTorqueAtRoot(glm::vec3(-0.005, 0, 0));
        posePhysicsBody.applyImpulseAtRoot(glm::vec3(0, 0, -0.05));
        box.body.body->addTorque(PxVec3(-0.05, 0, 0), PxForceMode::eIMPULSE, true);
    }

    void processInput(SDL_Event &event) override {
    }

    void update(float dt) override {
        static float time = 0.f;
        const float physicsDt = 1.0f / 120.0f;

        time += dt;


        auto inputMgr = InputManager::get();
        if (inputMgr->isKeyEntered(SDL_SCANCODE_SPACE)) {
            enablePhysics = !enablePhysics;
        }
        if (inputMgr->isKeyEntered(SDL_SCANCODE_F1)) {
            enableDebugRendering = !enableDebugRendering;
        }
        if (inputMgr->isKeyEntered(SDL_SCANCODE_F2)) {
            renderSimple = !renderSimple;
        }

        if (inputMgr->isKeyEntered(SDL_SCANCODE_RETURN)) {
            startRagdoll();
        }
        if (inputMgr->isKeyPressed(SDL_SCANCODE_UP))
            animFSM.setParam("is_walking", true);
        else
            animFSM.setParam("is_walking", false);

        // Update animation
        if (!enableRagdoll) {
            animFSM.update(dt);
            currentPose = animFSM.getCurrentPose();

            auto boxTrans = box.body.getTransform();
            boxLeftPos = boxTrans.v - glm::vec3(boxSize.x + 0.02f, 0, 0);
            boxRightPos = boxTrans.v + glm::vec3(boxSize.x + 0.02f, 0, 0);
            boxLeftPos.y += 2 * boxSize.y;
            boxRightPos.y += 2 * boxSize.y;

            // If animation controller arrives at hard-coded state, then start holding the box
            auto currentState = animFSM.getCurrentState();
            if (currentState && animFSM.get(currentState)->name == "pickup") {
                if (animFSM.getStateTime() >= 36.f / 30.f) {
                    isHoldingBox = true;
                }
            }

            if (isHoldingBox) {
                uint32_t leftHandIdx = poseTree.findIdx("LeftHand");
                uint32_t rightHandIdx = poseTree.findIdx("RightHand");
                auto leftHandTrans = calcFK(poseTree, currentPose, leftHandIdx);
                auto rightHandTrans = calcFK(poseTree, currentPose, rightHandIdx);
                glmx::transform boxTrans;
                boxTrans.v.x = 0.5f * (leftHandTrans.v.x + rightHandTrans.v.x);
                boxTrans.v.y = 0.5f * (leftHandTrans.v.y + rightHandTrans.v.y) - 2 * boxSize.y;
                if (boxTrans.v.y < 0.0f) boxTrans.v.y = 0.0f;
                boxTrans.v.z = currentPose.v.z - 0.4f;
                float theta = atan2(rightHandTrans.v.z - leftHandTrans.v.z, rightHandTrans.v.x - leftHandTrans.v.x);
                boxTrans.q = glm::rotate(-theta, glm::vec3(0, 1, 0));
                boxLeftPos = boxTrans.v - boxTrans.q * glm::vec3(boxSize.x + 0.02f, 0, 0);
                boxRightPos = boxTrans.v + boxTrans.q * glm::vec3(boxSize.x + 0.02f, 0, 0);
                boxLeftPos.y += 2 * boxSize.y;
                boxRightPos.y += 2 * boxSize.y;
                box.body.setTransform(boxTrans);

                solveTwoJointIK(poseTree, currentPose,
                                poseTree.findIdx("LeftArm"), poseTree.findIdx("LeftForeArm"),
                                poseTree.findIdx("LeftHand"),
                                boxLeftPos);

                solveTwoJointIK(poseTree, currentPose,
                                poseTree.findIdx("RightArm"), poseTree.findIdx("RightForeArm"),
                                poseTree.findIdx("RightHand"),
                                boxRightPos);
            }

            // If feet meets the obstacle (which is just hardcoded right now instead of proper collision check),
            // Then start ragdoll mode
            if (enableObstacle) {
                auto lfootTrans = calcFK(poseTree, currentPose, poseTree.findIdx("LeftFoot"));
                auto rfootTrans = calcFK(poseTree, currentPose, poseTree.findIdx("RightFoot"));
                auto obstaclePos = obstacle.body.getPosition();
                lfootTrans.v.x = rfootTrans.v.x = obstaclePos.x = 0.0f;
                if (glm::length(lfootTrans.v - obstaclePos) < obstacleSize.z / sqrt(2.f) ||
                    glm::length(rfootTrans.v - obstaclePos) < obstacleSize.z / sqrt(2.f)) {
                    startRagdoll();
                }
            }
        }

        // Update physics
        while (time >= physicsDt) {
            time -= physicsDt;

            if (enableManipulation) {
                posePhysicsBody.setPose(currentPose, poseTree);
                if (enablePhysics) {
                    bool advanced = world.advance(physicsDt);
                    if (advanced) {
                        world.fetchResults();
                    }
                }
                // posePhysicsBody.getPose(convertedPose, poseTree);
            }
            else if (enableRagdoll) {
                if (enablePhysics) {
                    bool advanced = world.advance(physicsDt);
                    if (advanced) {
                        world.fetchResults();
                    }
                }
                posePhysicsBody.getPose(currentPose, poseTree);
            }
            else {
                posePhysicsBody.setPose(currentPose, poseTree);
                if (enablePhysics) {
                    bool advanced = world.advance(physicsDt);
                    if (advanced) {
                        world.fetchResults();
                    }
                }
            }
        }

        // Update the camera
        if (isCameraFixed && !enableRagdoll) {
            FlyCamera* flyCamera = dynamic_cast<FlyCamera*>(camera.get());
            glm::vec3 cameraPos = currentPose.v;
            cameraPos.y += 0.7f;
            cameraPos.x += 2.5f;
            flyCamera->transform->setPosition(cameraPos);
            flyCamera->transform->setRotation(glm::rotate((float)M_PI/2, glm::vec3(0, 1, 0)));
        }
    }


    void render() override {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        pbRenderer.queueRender({groundMesh, groundMat, rootTransform->getWorldTransform()});
        if (renderSimple) {
            renderMotionClip(pbRenderer, imRenderer, currentPose, poseTree, poseRenderBodySimple);
        }
        else {
            renderMotionClipComplex(pbRenderer, imRenderer, currentPose, poseTree, poseRenderBody);
        }

        if (enableBox) {
            auto boxTrans = box.body.getTransform();
            pbRenderer.queueRender({box.mesh, box.material, glm::translate(boxTrans.v) * glm::mat4_cast(boxTrans.q)});
        }
        if (enableSpheres) {
            for (auto& sphere : spheres) {
                pbRenderer.queueRender({sphere.mesh, sphere.material, glm::translate(sphere.body.getTransform().v)});
            }
        }
        if (enableObstacle) {
            pbRenderer.queueRender({obstacle.mesh, obstacle.material,
                                    glm::translate(obstacle.body.getPosition()) * glm::mat4_cast(obstacle.body.getRotation())});
        }
        pbRenderer.render();

        for (int i = 0; i < pbRenderer.pointLights.size(); i++) {
            if (pbRenderer.pointLights[i].enabled) {
                imRenderer.drawPoint(pbRenderer.pointLights[i].position, colors::Yellow, 4.0f, true);
            }
        }

        if (enableDebugRendering) {
            imRenderer.drawAxisTriad(glm::mat4(1.0f), 0.1f, 1.0f, false);
            imRenderer.drawSphere(boxLeftPos, colors::Blue, 0.05f, true);
            imRenderer.drawSphere(boxRightPos, colors::Blue, 0.05f, true);
            glm::vec3 leftHandPos = calcFK(poseTree, currentPose, poseTree.findIdx("LeftHand")).v;
            glm::vec3 rightHandPos = calcFK(poseTree, currentPose, poseTree.findIdx("RightHand")).v;
            imRenderer.drawSphere(leftHandPos, colors::Green, 0.05f, true);
            imRenderer.drawSphere(rightHandPos, colors::Green, 0.05f, true);
            imRenderer.render();
        }

        if (enableDebugRendering) {
            pxDebugRenderer.render(world);
        }

        renderImGui();
    }


    void renderImGui() {
        animFSM.renderImGui(poseTree);

        ImGui::Begin("Character Data");

        ImGui::Checkbox("Enable Manipulation", &enableManipulation);
        if (!enableManipulation) {
            if (ImGui::Button("Ragdoll")) {
                startRagdoll();
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset")) {
                reset();
            }
        }

        if (ImGui::TreeNode("Kinematics")) {
            ImGui::DragFloat3((poseTree[0].name + " pos").c_str(), (float*)&currentPose.v, 0.01f);
            for (uint32_t i = 0; i < poseTree.numJoints; i++) {
                auto& node = poseTree[i];
                glm::vec3 v = glmx::quatToEuler(currentPose.q[i], EulOrdZYXs);
                if (ImGui::DragFloat3(node.name.c_str(), (float*)&v, 0.01f)) {
                    currentPose.q[i] = glmx::eulerToQuat(v);
                }
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("PhysX")) {
            posePhysicsBody.renderImGui();
            ImGui::TreePop();
        }

        ImGui::End();
    }

    void release() override {
        world.release();
    }

private:
    const int N = 7;

    glmx::pose currentPose;
    PoseTree poseTree;
    BVHData pickupBVH;
    AnimStateMachine animFSM;
    Ref<AnimState> pickupState;

    PoseRenderBodyPBR poseRenderBody;
    PoseRenderBodyPBR poseRenderBodySimple;

    Ref<Mesh> groundMesh;
    Ref<PBRMaterial> groundMat;

    PhysicsObject obstacle;

    PxFoundation* pxFoundation;
    PhysicsWorld world;
    PhysXDebugRenderer pxDebugRenderer;
    PosePhysicsBody posePhysicsBody;
    PosePhysicsBodySkel posePhysicsBodySkel;

    const int sphereColorsCount = 10;
    const int sphereCountWidth = 5;
    const int sphereCount = sphereCountWidth * sphereCountWidth * sphereCountWidth;
    const float sphereRadius = 0.035f;
    const float sphereDist = 0.001f;

    glm::vec3 boxSize = {0.25f, 0.15f, 0.25f};
    float boxThickness = 0.025f;
    glm::vec3 obstacleSize = {4.0f, 0.1f, 0.1f};

    PhysicsObject box;
    std::vector<PhysicsObject> spheres;

    bool enableDebugRendering = true;
    bool renderSimple = true;

    bool enableRagdoll = false;
    bool enableManipulation = false;
    bool enablePhysics = true;

    bool enableBox = true;
    bool enableSpheres = true;
    bool enableObstacle = true;

    bool isHoldingBox = false;
    bool isCameraFixed = false;

    glm::vec3 boxLeftPos, boxRightPos;
};

int main(int argc, char** argv) {
    MyApp app;
    app.load();
    app.startMainLoop();
    app.release();

    return 0;
}

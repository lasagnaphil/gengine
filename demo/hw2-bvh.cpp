//
// Created by lasagnaphil on 19. 9. 23..
//

#include <InputManager.h>
#include "App.h"
#include "PhongRenderer.h"
#include "GizmosRenderer.h"
#include "FlyCamera.h"
#include "glmx/pose.h"
#include "PoseRenderBody.h"
#include "MotionClipData.h"
#include "MotionClipPlayer.h"

#include <map>
#include <glm/gtx/euler_angles.hpp>


class MyApp : public App {
public:
    MyApp() : App(true) {}

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
        motionClipData = MotionClipData::loadFromFile("resources/cmu_07_02_1.bvh", 0.01f);
        motionClipData.removeCMUPhantomJoints();

        if (!motionClipData.valid)
        {
            std::cerr << "BVH Not Found!" << std::endl;
            exit(1);
        }
        motionClipPlayer = BVHMotionClipPlayer(&motionClipData);

        // Material of human
        Ref<PhongMaterial> bodyMat = Resources::make<PhongMaterial>();
        bodyMat->ambient = {0.1f, 0.1f, 0.1f, 1.0f};
        bodyMat->specular = {0.7f, 0.7f, 0.7f, 1.0f};
        bodyMat->diffuse = {1.0f, 0.0f, 0.0f, 1.0f};
        bodyMat->shininess = 64.0f;
        bodyMat->texDiffuse = {};
        bodyMat->texSpecular = {};

        // Create body using capsules
        poseRenderBody = PoseRenderBody::createAsBoxes(motionClipData.poseTree, 0.05f, bodyMat);
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
        motionClipPlayer.update(dt);
    }

    void render() override {
        phongRenderer.queueRender({groundMesh, groundMat, rootTransform->getWorldTransform()});
        renderMotionClip(phongRenderer, imRenderer, motionClipPlayer.getPoseState(), motionClipData.poseTree, poseRenderBody);
        phongRenderer.render();

        gizmosRenderer.render();

        phongRenderer.renderImGui();
        motionClipPlayer.renderImGui();
    }

    void release() override {
    }

private:
    MotionClipData motionClipData;
    BVHMotionClipPlayer motionClipPlayer;
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
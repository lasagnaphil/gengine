//
// Created by lasagnaphil on 19. 4. 30.
//

#define _DEBUG

#include <iostream>
#include <InputManager.h>
#include "App.h"
#include "PBRenderer.h"
#include "GizmosRenderer.h"
#include "FlyCamera.h"

class MyApp : public App {
public:
    MyApp() : App(false, AppRenderSettings::PBR) {}

    void loadResources() override {
        FlyCamera* camera = initCamera<FlyCamera>();
        Ref<Transform> cameraTransform = camera->transform;
        cameraTransform->move({0.0f, 3.0f, 0.0f});

        groundMat = PBRMaterial::quick(
                "resources/textures/mossy-ground1-albedo.png",
                "resources/textures/mossy-ground1-metal.png",
                "resources/textures/mossy-ground1-roughness.png",
                "resources/textures/mossy-ground1-ao.png");

        groundMesh = Mesh::makePlane(1000.0f, 100.0f);

        sphereMat = PBRMaterial::quick(
                "resources/textures/rustediron2_basecolor.png",
                "resources/textures/rustediron2_metallic.png",
                "resources/textures/rustediron2_roughness.png",
                "resources/textures/rustediron2_ao.png");

        sphereTransforms.resize(10 * 10);

        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                int id = 10*i + j;
                sphereTransforms[id] = Resources::make<Transform>();
                Transform::addChildToParent(sphereTransforms[id], rootTransform);
                sphereTransforms[id]->setPosition({0.0f, (float)i, (float)j});
                sphereTransforms[id]->setScale({1.0f, 1.0f, 1.0f});
            }
        }

        sphereMesh = Mesh::makeSphere();
    }

    void processInput(SDL_Event &event) override {
    }

    void update(float dt) override {
        auto inputMgr = InputManager::get();
        if (inputMgr->isKeyEntered(SDL_SCANCODE_1)) {
            phongRenderer.viewDepthBufferDebug = !phongRenderer.viewDepthBufferDebug;
        }
    }

    void render() override {
        pbRenderer.queueRender({groundMesh, groundMat, rootTransform->getWorldTransform()});
        for (auto& transform : sphereTransforms) {
            pbRenderer.queueRender({sphereMesh, sphereMat, transform->getWorldTransform()});
        }
        pbRenderer.render();

        gizmosRenderer.render();
    }

    void release() override {
    }

private:
    Ref<PBRMaterial> groundMat;
    Ref<PBRMaterial> sphereMat;
    Ref<Mesh> groundMesh;
    Ref<Mesh> sphereMesh;
    std::vector<Ref<Transform>> sphereTransforms;
};

int main(int argc, char** argv) {
    MyApp app;
    app.start();

    return 0;
}
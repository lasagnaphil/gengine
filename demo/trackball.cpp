//
// Created by lasagnaphil on 19. 4. 30.
//

#define _DEBUG

#include <iostream>
#include <InputManager.h>
#include "App.h"
#include "PhongRenderer.h"
#include "GizmosRenderer.h"
#include "TrackballCamera.h"

class MyApp : public App {
public:
    MyApp() : App(false) {}

    void loadResources() override {
        TrackballCamera* camera = initCamera<TrackballCamera>();

        Ref<Image> checkerImage = Resources::make<Image>("resources/textures/checker.png");
        Ref<Texture> planeTexture = Texture::fromImage(checkerImage);
        checkerImage.release();

        groundMat = Resources::make<PhongMaterial>();
        groundMat->ambient = {0.1f, 0.1f, 0.1f, 1.0f};
        groundMat->specular = {0.7f, 0.7f, 0.7f, 1.0f};
        groundMat->shininess = 32.0f;
        groundMat->texDiffuse = planeTexture;
        groundMat->texSpecular = {};

        groundMesh = Mesh::makePlane(1000.0f, 100.0f);

        Ref<Image> cubeImage = Resources::make<Image>("resources/textures/container2.png");
        Ref<Texture> cubeTexture = Texture::fromImage(cubeImage);
        cubeImage.release();

        Ref<Image> cubeSpecularImage = Resources::make<Image>("resources/textures/container2_specular.png");
        Ref<Texture> cubeSpecularTexture = Texture::fromImage(cubeSpecularImage);
        cubeSpecularImage.release();

        cubeMat = Resources::make<PhongMaterial>();
        cubeMat->ambient = {0.0f, 0.0f, 0.0f, 1.0f};
        cubeMat->shininess = 32.0f;
        cubeMat->texDiffuse = cubeTexture;
        cubeMat->texSpecular = cubeSpecularTexture;

        cubeTransform = Resources::make<Transform>();
        Transform::addChildToParent(cubeTransform, rootTransform);
        cubeTransform->setPosition({0.f, 0.f, 0.f});
        cubeTransform->setScale({5.0f, 5.0f, 5.0f});

        cubeMesh = Mesh::makeCube();
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
        // phongRenderer.queueRender({groundMesh, groundMat, rootTransform->getWorldTransform()});
        phongRenderer.queueRender({cubeMesh, cubeMat, cubeTransform->getWorldTransform()});
        phongRenderer.render();

        gizmosRenderer.render();
    }

    void release() override {
    }

private:
    Ref<PhongMaterial> groundMat;
    Ref<PhongMaterial> cubeMat;
    Ref<Mesh> groundMesh;
    Ref<Mesh> cubeMesh;

    Ref<Transform> cubeTransform;
};

int main(int argc, char** argv) {
    MyApp app;
    app.start();

    return 0;
}
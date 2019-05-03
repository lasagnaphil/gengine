//
// Created by lasagnaphil on 19. 4. 30.
//

#define _DEBUG

#include <iostream>
#include <InputManager.h>
#include "App.h"
#include "PhongRenderer.h"

class MyApp : public App {
public:
    MyApp() : App(false) {}

    void loadResources() override {
        Ref<Transform> cameraTransform = trackballCamera.transform;
        cameraTransform->move({10.0f, 20.0f, -10.0f});
        cameraTransform->rotate(M_PI/4, {0.0f, 0.0f, 1.0f});

        Ref<Image> checkerImage = Resources::make<Image>("gengine/resources/textures/checker.png");
        Ref<Texture> planeTexture = Resources::make<Texture>(checkerImage);
        checkerImage.release();

        groundMat = Resources::make<Material>();
        groundMat->ambient = {0.1f, 0.1f, 0.1f, 1.0f};
        groundMat->specular = {0.7f, 0.7f, 0.7f, 1.0f};
        groundMat->shininess = 32.0f;
        groundMat->texDiffuse = planeTexture;
        groundMat->texSpecular = {};

        groundMesh = Mesh::makePlane(50.0f, 5.0f);
        groundMesh->initVBO();

        Ref<Image> cubeImage = Resources::make<Image>("gengine/resources/textures/container2.png");
        Ref<Texture> cubeTexture = Resources::make<Texture>(cubeImage);
        cubeImage.release();

        Ref<Image> cubeSpecularImage = Resources::make<Image>("gengine/resources/textures/container2_specular.png");
        Ref<Texture> cubeSpecularTexture = Resources::make<Texture>(cubeSpecularImage);
        cubeSpecularImage.release();

        cubeMat = Resources::make<Material>();
        cubeMat->ambient = {0.0f, 0.0f, 0.0f, 1.0f};
        cubeMat->shininess = 32.0f;
        cubeMat->texDiffuse = cubeTexture;
        cubeMat->texSpecular = cubeSpecularTexture;

        cubeTransforms.resize(3);

        cubeTransforms[0] = Resources::make<Transform>();
        Transform::addChildToParent(cubeTransforms[0], rootTransform);
        cubeTransforms[0]->setPosition({6.0f, 2.0f, -4.0f});
        cubeTransforms[0]->setScale({4.0f, 4.0f, 4.0f});

        cubeTransforms[1] = Resources::make<Transform>();
        Transform::addChildToParent(cubeTransforms[1], rootTransform);
        cubeTransforms[1]->setPosition({0.0f, 3.0f, 10.0f});
        cubeTransforms[1]->setScale({6.0f, 6.0f, 6.0f});

        cubeTransforms[2] = Resources::make<Transform>();
        Transform::addChildToParent(cubeTransforms[2], rootTransform);
        cubeTransforms[2]->setPosition({0.0f, 4.0f, -6.0f});
        cubeTransforms[2]->setScale({8.0f, 8.0f, 8.0f});

        cubeMesh = Mesh::makeCube();
        cubeMesh->initVBO();
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
        phongRenderer.queueRender({groundMesh, groundMat, rootTransform->getWorldTransform()});
        for (auto& transform : cubeTransforms) {
            phongRenderer.queueRender({cubeMesh, cubeMat, transform->getWorldTransform()});
        }
        phongRenderer.render();
    }

    void release() override {
    }

private:
    Ref<Material> groundMat;
    Ref<Material> cubeMat;
    Ref<Mesh> groundMesh;
    Ref<Mesh> cubeMesh;
    std::vector<Ref<Transform>> cubeTransforms;
};

int main(int argc, char** argv) {
    MyApp app;
    app.start();

    return 0;
}
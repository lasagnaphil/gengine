//
// Created by lasagnaphil on 19. 4. 30.
//

#define _DEBUG

#include <iostream>
#include "App.h"
#include "PhongRenderer.h"

class MyApp : public App {
public:
    MyApp() : App(false) {}

    void loadResources() override {
        Ref<Image> checkerImage = Resources::make<Image>("gengine/resources/textures/checker.png");
        Ref<Texture> planeTexture = Resources::make<Texture>(checkerImage);
        checkerImage.release();

        groundMat = Resources::make<Material>();
        groundMat->ambient = {0.3f, 0.3f, 0.3f, 1.0f};
        groundMat->diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
        groundMat->specular = {0.7f, 0.7f, 0.7f, 1.0f};
        groundMat->shininess = 32.0f;
        groundMat->texDiffuse = planeTexture;
        groundMat->texSpecular = {};

        groundMesh = Mesh::makePlane(1000.0f, 100.0f);
        groundMesh->initVBO();

        cubeMat = Resources::make<Material>();
        cubeMat->ambient = {0.3f, 0.3f, 0.3f, 1.0f};
        cubeMat->diffuse = {1.0f, 0.0f, 0.0f, 1.0f};
        cubeMat->specular = {1.0f, 0.0f, 0.0f, 1.0f};
        cubeMat->shininess = 32.0f;
        cubeMat->texDiffuse = {};
        cubeMat->texSpecular = {};

        cubeTransform = Resources::make<Transform>();
        Transform::addChildToParent(cubeTransform, rootTransform);
        cubeTransform->setPosition({0.0f, 5.0f, 0.0f});
        cubeTransform->setScale({4.0f, 4.0f, 4.0f});

        cubeMesh = Mesh::makeCube();
        cubeMesh->initVBO();
    }

    void processInput(SDL_Event &event) override {

    }

    void update(float dt) override {

    }

    void render() override {
        phongRenderer.queueRender({cubeMesh, cubeMat, cubeTransform->getWorldTransform()});
        phongRenderer.queueRender({groundMesh, groundMat, rootTransform->getWorldTransform()});
        phongRenderer.render();
    }

    void release() override {
    }

private:
    Ref<Material> groundMat;
    Ref<Material> cubeMat;
    Ref<Mesh> groundMesh;
    Ref<Mesh> cubeMesh;
    Ref<Transform> cubeTransform;
};

int main(int argc, char** argv) {
    MyApp app;
    app.start();

    return 0;
}
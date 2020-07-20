//
// Created by lasagnaphil on 19. 4. 30.
//

#define _DEBUG

#include <iostream>
#include "gengine/InputManager.h"
#include "gengine/App.h"
#include "gengine/PhongRenderer.h"
#include "gengine/GizmosRenderer.h"
#include "gengine/FlyCamera.h"

class MyApp : public App {
public:
    MyApp() : App() {}

    void loadResources() override {
        FlyCamera* camera = dynamic_cast<FlyCamera*>(this->camera.get());
        Ref<Transform> cameraTransform = camera->transform;
        cameraTransform->move({-10.0f, 20.0f, 10.0f});

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

        Ref<Image> cubeImage = Image::fromFile("resources/textures/container2.png");
        Ref<Texture> cubeTexture = Texture::fromImage(cubeImage);
        cubeImage.release();

        Ref<Image> cubeSpecularImage = Image::fromFile("resources/textures/container2_specular.png");
        Ref<Texture> cubeSpecularTexture = Texture::fromImage(cubeSpecularImage);
        cubeSpecularImage.release();

        cubeMat = Resources::make<PhongMaterial>();
        cubeMat->ambient = {0.0f, 0.0f, 0.0f, 1.0f};
        cubeMat->shininess = 32.0f;
        cubeMat->texDiffuse = cubeTexture;
        cubeMat->texSpecular = cubeSpecularTexture;

        sphereTransforms.resize(3);

        sphereTransforms[0] = Resources::make<Transform>();
        Transform::addChildToParent(sphereTransforms[0], rootTransform);
        sphereTransforms[0]->setPosition({6.0f, 3.0f, -4.0f});
        sphereTransforms[0]->setScale({6.0f, 6.0f, 6.0f});

        sphereTransforms[1] = Resources::make<Transform>();
        Transform::addChildToParent(sphereTransforms[1], rootTransform);
        sphereTransforms[1]->setPosition({0.0f, 10.0f, 10.0f});
        sphereTransforms[1]->setScale({6.0f, 6.0f, 6.0f});

        sphereTransforms[2] = Resources::make<Transform>();
        Transform::addChildToParent(sphereTransforms[2], rootTransform);
        sphereTransforms[2]->setPosition({0.0f, 12.0f, -2.0f});
        sphereTransforms[2]->setScale({6.0f, 6.0f, 6.0f});

        cubeMesh = Mesh::makeCube();

        std::vector<glm::vec3> positions = {
                {0.0f, 0.0f, 0.0f},
                {0.0f, 5.0f, 0.0f},
                {5.0f, 5.0f, 0.0f},
        };

        lineMesh = Resources::make<LineMesh>(positions);
        lineMesh->subdivisionBSpline(3);
        lineMesh->init();
        lineMat = Resources::make<LineMaterial>();
        lineMat->lineType = GL_LINE_STRIP;
        lineMat->drawLines = true;
        lineMat->drawPoints = true;
        lineMat->lineColor = {1.0f, 0.0f, 0.0f, 1.0f};
        lineMat->lineWidth = 1.0f;
        lineMat->pointColor = {1.0f, 0.0f, 0.0f, 1.0f};
        lineMat->pointSize = 3.0f;
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
        for (auto& transform : sphereTransforms) {
            phongRenderer.queueRender({cubeMesh, cubeMat, transform->getWorldTransform()});
        }
        phongRenderer.render();

        gizmosRenderer.queueLine({lineMesh, lineMat, glm::mat4(1.0f)});
        gizmosRenderer.render();
    }

    void release() override {
    }

private:
    Ref<PhongMaterial> groundMat;
    Ref<PhongMaterial> cubeMat;
    Ref<Mesh> groundMesh;
    Ref<Mesh> cubeMesh;
    Ref<LineMesh> lineMesh;
    Ref<LineMaterial> lineMat;
    std::vector<Ref<Transform>> sphereTransforms;
};

int main(int argc, char** argv) {
    MyApp app;
    app.load();
    app.startMainLoop();
    app.release();

    return 0;
}
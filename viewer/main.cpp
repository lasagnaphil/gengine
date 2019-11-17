//
// Created by lasagnaphil on 19. 4. 30.
//

#define _DEBUG

#include <iostream>
#include <InputManager.h>
#include <MotionClipData.h>
#include <MotionClipPlayer.h>
#include "ezc3d.h"
#include <C3DPlayer.h>

#include "App.h"
#include "PhongRenderer.h"
#include "GizmosRenderer.h"
#include "TrackballCamera.h"
#include "portable_file_dialogs.h"


class ViewerApp : public App {
public:
    void printUsage(const char* execName) {
        std::cout << "Usage: " << std::endl;
        std::cout << "\t" << execName << "<filename>" << std::endl;
    }

    ViewerApp(int argc, char** argv) : App(false) {
        if (argc >= 3) {
            printUsage(argv[0]);
            exit(EXIT_FAILURE);
        }

        if (argc == 2) {
            auto filepath = std::string(argv[1]);
            auto dpos = filepath.find('.');
            auto extension = filepath.substr(dpos + 1);
            if (extension == "bvh") {
                MotionClipData::loadFromFile(filepath, motionClipData);
                isBvhLoaded = true;
            }
            else if (extension == "c3d") {
                c3d = ezc3d::c3d(filepath);
                c3dPlayer = C3DPlayer(&c3d);
                c3dPlayer.init();
                isC3dLoaded = true;
            }
            else {
                printUsage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }

    bool isBvhLoaded = false;
    bool isC3dLoaded = false;

    void loadResources() override {
        TrackballCamera* trackballCamera = initCamera<TrackballCamera>();
        // flyCamera->movementSpeed = 100.0f;
        // flyCamera->far = 100000.0f;

        Ref<Transform> cameraTransform = trackballCamera->transform;
        cameraTransform->move({10.0f, 20.0f, -10.0f});
        cameraTransform->rotate(M_PI/4, {0.0f, 0.0f, 1.0f});

        Ref<Image> checkerImage = Image::fromFile("resources/textures/checker.png");
        Ref<Texture> planeTexture = Texture::fromImage(checkerImage);
        checkerImage.reset();

        groundMat = Resources::make<PhongMaterial>();
        groundMat->ambient = {0.1f, 0.1f, 0.1f, 1.0f};
        groundMat->specular = {0.7f, 0.7f, 0.7f, 1.0f};
        groundMat->shininess = 32.0f;
        groundMat->texDiffuse = planeTexture;
        groundMat->texSpecular = {};

        groundMesh = Mesh::makePlane(1000.0f, 10.0f);
    }

    void processInput(SDL_Event &event) override {
    }

    void update(float dt) override {
        auto inputMgr = InputManager::get();
    }

    void render() override {
        phongRenderer.queueRender({groundMesh, groundMat, rootTransform->getWorldTransform()});
        phongRenderer.render();

        if (isC3dLoaded) {
            c3dPlayer.queueGizmosRender(gizmosRenderer, glm::mat4(1.0f));
        }
        gizmosRenderer.render();

        drawImGui();
        if (isC3dLoaded) {
            c3dPlayer.drawImGui();
        }
    }

    void release() override {
    }

    void drawImGui() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Load")) {
                    auto selection = pfd::open_file("Select a file", ".",
                                                    { "BVH Files", "*.bvh",
                                                      "C3D Files", "*.c3d"}, true).result();
                    if (selection.size() == 1) {
                        auto filepath = selection[0];
                        auto dpos = filepath.find('.');
                        auto extension = filepath.substr(dpos + 1);
                        if (extension == "bvh") {
                            if (!MotionClipData::loadFromFile(filepath, motionClipData)) {
                                isBvhLoaded = true;
                            }
                            else {
                                // TODO: print error messazge
                            }

                        }
                        else if (extension == "c3d") {
                            try {
                                c3d = ezc3d::c3d(filepath);
                                c3dPlayer = C3DPlayer(&c3d);
                                c3dPlayer.init();
                                isC3dLoaded = true;
                            } catch (std::exception& ex) {
                                // TODO: print error message
                            }

                        }
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

private:
    Ref<PhongMaterial> groundMat;
    Ref<Mesh> groundMesh;

    MotionClipData motionClipData;
    MotionClipPlayer motionClipPlayer;

    ezc3d::c3d c3d;
    C3DPlayer c3dPlayer;
};

int main(int argc, char** argv) {
    ViewerApp app(argc, argv);
    app.start();

    return 0;
}
//
// Created by lasagnaphil on 19. 8. 7.
//

#ifndef GENGINE_C3DPLAYER_H
#define GENGINE_C3DPLAYER_H

#include "ezc3d.h"
#include "LineMesh.h"
#include "GizmosRenderer.h"

struct C3DPlayer {
    C3DPlayer() : c3d(nullptr) {}
    C3DPlayer(ezc3d::c3d* c3d) : c3d(c3d) {}

    ezc3d::c3d* c3d;

    int currentFrameIdx = 0;
    bool isPlaying = false;
    int numFrames = 0;
    int numPoints = 0;

    Ref<LineMesh> mesh;
    Ref<LineMaterial> mat;

    void init() {
        assert(c3d != nullptr);
        numFrames = c3d->data().nbFrames();
        numPoints = c3d->data().frame(0).points().nbPoints();

        std::vector<glm::vec3> markerPositions(numPoints);
        for (int i = 0; i < numPoints; i++) {
            auto point = c3d->data().frame(currentFrameIdx).points().point(i);
            markerPositions[i] = {point.x(), point.y(), point.z()};
        }

        mesh = Resources::make<LineMesh>(markerPositions);
        mesh->init();

        mat = Resources::make<LineMaterial>();
        mat->drawLines = false;
        mat->drawPoints = true;
        mat->pointColor = {1.0f, 0.0f, 0.0f, 1.0f};
        mat->pointSize = 4.0f;
    }

    void play() {
        isPlaying = true;
    }

    void pause() {
        isPlaying = false;
    }

    void stop() {
        isPlaying = false;
        currentFrameIdx = 0;
    }
    void previousFrame() {
        currentFrameIdx--;
        if (currentFrameIdx == -1) {
            currentFrameIdx = numFrames - 1;
        }
        updateMesh();
    }

    void nextFrame() {
        currentFrameIdx++;
        if (currentFrameIdx >= numFrames) {
            currentFrameIdx = 0;
        }
        updateMesh();
    }

    void update(float dt) {
        static float frameUpdateLag = 0.0f;

        if (isPlaying) {
            frameUpdateLag += dt;
            while (frameUpdateLag >= c3d->header().frameRate()) {
                nextFrame();
                frameUpdateLag -= c3d->header().frameRate();
            }
        }
        else {
            frameUpdateLag = 0.0f;
        }
    }

    void updateMesh() {
        for (int i = 0; i < numPoints; i++) {
            auto point = c3d->data().frame(currentFrameIdx).points().point(i);
            mesh->positions[i] = {point.x(), point.y(), point.z()};
        }
        mesh->updateBuffers();
    }

    void queueGizmosRender(GizmosRenderer& renderer, glm::mat4 modelMatrix) {
        renderer.queueLine({mesh, mat, modelMatrix});
    }

    void drawImGui() {
        ImGui::Begin("C3D Player");

        ImGui::Text("Current Frame: %d", currentFrameIdx);
        if (ImGui::ArrowButton("Left##PoseMesh", ImGuiDir_Left)) {
            previousFrame();
        }
        ImGui::SameLine();
        if (ImGui::ArrowButton("Right##PoseMesh", ImGuiDir_Right)) {
            nextFrame();
        }
        ImGui::SameLine();
        if (ImGui::Button("Play##PoseMesh")) {
            play();
        }
        ImGui::SameLine();
        if (ImGui::Button("Pause##PoseMesh")) {
            pause();
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop##PoseMesh")) {
            stop();
        }

        if (ImGui::SliderInt("Frame##PoseMesh frame slider", &currentFrameIdx, 0, numFrames - 1)) {
            updateMesh();
        }

        ImGui::End();
    }
};


#endif //GENGINE_C3DPLAYER_H

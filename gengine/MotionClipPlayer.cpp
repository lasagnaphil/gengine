//
// Created by lasagnaphil on 19. 5. 9.
//

#include "MotionClipPlayer.h"

#include "GizmosRenderer.h"

void MotionClipPlayer::update(float dt) {
    static float frameUpdateLag = 0.0f;
    static float motionUpdateLag = 0.0f;
    if (isPlaying) {
        frameUpdateLag += dt;
        while (frameUpdateLag >= getFrameTime()) {
            nextFrame();
            frameUpdateLag -= getFrameTime();
        }
    }
    else {
        frameUpdateLag = 0.0f;
    }
}

void MotionClipPlayer::renderImGui() {
    std::string windowName = fmt::format("Motion Clip Player ({})", getName());
    ImGui::Begin(windowName.c_str());

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

    ImGui::SliderInt("Frame##PoseMesh frame slider", &currentFrameIdx, 0, getNumFrames() - 1);

    ImGui::End();
}

int BVHMotionClipPlayer::counter = 0;

void BVHMotionClipPlayer::renderImGui() {
    MotionClipPlayer::renderImGui();

    // TODO: Display BVH joint data
}

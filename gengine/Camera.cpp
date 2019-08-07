//
// Created by lasagnaphil on 4/7/18.
//

#include <imgui.h>
#include <SDL2/SDL_events.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/quaternion.hpp>

#include "GenAllocator.h"
#include "Camera.h"
#include "Transform.h"
#include "InputManager.h"
#include "imgui_impl_sdl.h"

Camera::Camera(Ref<Transform> parent)
{
    transform = Resources::make<Transform>();
    transform->setPosition(glm::vec3(0.f, 0.f, distance));
    Transform::addChildToParent(transform, parent);
    /*
    trackballFocus = Resources::insert(Transform {});
    Transform::addChildToParent(transform, trackballFocus);
    Transform::addChildToParent(trackballFocus, parent);
     */

    viewport = {0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y};
    updateCameraVectors();
}

void Camera::update(float dt) {
    auto inputMgr = InputManager::get();

    bool pressStart = false;
    if (inputMgr->isMouseEntered(SDL_BUTTON_RIGHT)) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        pressStart = true;
    }
    if (inputMgr->isMousePressed(SDL_BUTTON_RIGHT)) {
        // FPS Controls
        auto mouseOffsetI = inputMgr->getRelMousePos();
        auto mouseOffset = pressStart? glm::vec2() : glm::vec2((float) mouseOffsetI.x, (float) mouseOffsetI.y);

        mouseOffset *= mouseSensitivity;

        yaw += mouseOffset.x;
        pitch += mouseOffset.y;

        if (constrainPitch) {
            if (pitch > 89.0f) pitch = 89.0f;
            if (pitch < -89.0f) pitch = -89.0f;
        }

        updateCameraVectors();

        // Keyboard movement
        float velocity = movementSpeed * dt;
        if (inputMgr->isKeyPressed(SDL_SCANCODE_W)) {
            transform->move(-transform->getFrontVec() * velocity);
        }
        else if (inputMgr->isKeyPressed(SDL_SCANCODE_S)) {
            transform->move(transform->getFrontVec() * velocity);
        }
        if (inputMgr->isKeyPressed(SDL_SCANCODE_A)) {
            transform->move(-transform->getRightVec() * velocity);
        }
        else if (inputMgr->isKeyPressed(SDL_SCANCODE_D)) {
            transform->move(transform->getRightVec() * velocity);
        }
        if (inputMgr->isKeyPressed(SDL_SCANCODE_Q)) {
            transform->move(transform->getUpVec() * velocity);
        }
        else if (inputMgr->isKeyPressed(SDL_SCANCODE_E)) {
            transform->move(-transform->getUpVec() * velocity);
        }
    }
    else if (inputMgr->isMousePressed(SDL_BUTTON_MIDDLE)) {
        // Trackball controls are crap; disable it
        /*
        static glm::ivec2 refMousePos = {};
        static glm::ivec2 prevMousePos = {};
        static glm::ivec2 mousePos = {};

        auto displaySize = ImGui::GetIO().DisplaySize;
        glm::vec3 vref;

        if (inputMgr->isMouseEntered(SDL_BUTTON_MIDDLE)) {
            refMousePos = inputMgr->getMousePos();
            mousePos = refMousePos;
            prevMousePos = refMousePos;
            auto mref = glm::vec2 {refMousePos.x - displaySize.x/2, -refMousePos.y + displaySize.y/2};
            vref = calcMouseVec(mref);
        }

        mousePos = inputMgr->getMousePos();

        auto m1 = glm::vec2 {prevMousePos.x - displaySize.x/2, -prevMousePos.y + displaySize.y/2};
        auto m2 = glm::vec2 {mousePos.x - displaySize.x/2, -mousePos.y + displaySize.y/2};

        glm::vec3 v1 = calcMouseVec(m1);
        glm::vec3 v2 = calcMouseVec(m2);

        glm::quat q;
        float dotProduct = glm::dot(v2, v1);
        float lengthProduct = std::sqrt(glm::length2(v1) * glm::length2(v2));
        if (dotProduct / lengthProduct == -1) {
            q = glm::quat(1.f, 0.f, 0.f, 0.f);
        } else {
            q = glm::quat(lengthProduct + dotProduct, glm::cross(v2, v1));
            q = glm::normalize(q);
        }

        trackballFocus->rotateLocal(q);

        prevMousePos = mousePos;

        auto upDir = transform->getGlobalUpVec();
        auto rightDir = transform->getGlobalRightVec();
        if (inputMgr->isKeyPressed(SDL_SCANCODE_LEFT) || inputMgr->isKeyPressed(SDL_SCANCODE_A)) {
            trackballFocus->moveGlobal(-translationSpeed * dt * rightDir);
        }
        else if (inputMgr->isKeyPressed(SDL_SCANCODE_RIGHT) || inputMgr->isKeyPressed(SDL_SCANCODE_D)) {
            trackballFocus->moveGlobal(translationSpeed * dt * rightDir);
        }
        else if (inputMgr->isKeyPressed(SDL_SCANCODE_UP) || inputMgr->isKeyPressed(SDL_SCANCODE_W)) {
            trackballFocus->moveGlobal(translationSpeed * dt * upDir);
        }
        else if (inputMgr->isKeyPressed(SDL_SCANCODE_DOWN) || inputMgr->isKeyPressed(SDL_SCANCODE_S)) {
            trackballFocus->moveGlobal(-translationSpeed * dt * upDir);
        }
         */
    }
    else if (inputMgr->isMouseExited(SDL_BUTTON_RIGHT)) {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
}

void Camera::updateCameraVectors() {
    glm::quat quatX = glm::angleAxis(-glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat quatY = glm::angleAxis(-glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    transform->setRotation(glm::normalize(quatX * quatY));
}

void Camera::renderImGui() {
    auto trans = transform.get();
    // auto focusTrans = trackballFocus.get();

    /*
    ImGui::Begin("Trackball");
    ImGui::Text("Position of trackball focus: %s", glm::to_string(focusTrans->getGlobalPosition()).c_str());
    ImGui::Text("Position of trackball camera: %s", glm::to_string(trans->getGlobalPosition()).c_str());
    ImGui::Text("Theta: %f", glm::degrees(theta));
    ImGui::Text("GlobalFrontVec: %s", glm::to_string(trans->getGlobalFrontVec()).c_str());
    ImGui::Text("GlobalUpVec: %s", glm::to_string(trans->getGlobalUpVec()).c_str());
    ImGui::Text("Distance: %f", distance);
    ImGui::Text("Fov: %f", fov);
    ImGui::Checkbox("Use zoom instead of dolly", &enableZoom);
    ImGui::End();
     */
}

void Camera::processInput(SDL_Event& ev) {
    auto trans = transform.get();
    if (ev.type == SDL_MOUSEWHEEL) {
        if (enableZoom) {
            fov += ev.wheel.y;
        }
        else {
            float increment = 0.25f * ev.wheel.y;
            if (distance + increment > 0.f) {
                distance += increment;
            }
            auto curPos = trans->getPosition();
            auto nextPos = distance / glm::length(curPos) * curPos;
            trans->setPosition(nextPos);
        }
    }
}

Ray Camera::screenPointToRay(glm::vec2 mousePos) const {
    auto io = ImGui::GetIO();
    auto screenSize = glm::vec2 {io.DisplaySize.x, io.DisplaySize.y};

    // Thanks to http://antongerdelan.net/opengl/raycasting.html
    auto homogeneousCoord = glm::vec4 {
            2.0f * mousePos.x / screenSize.x - 1.0f,
            1.0f - 2.0f * mousePos.y / screenSize.y,
            -1.0f,
            1.0f};
    auto cameraCoord = glm::inverse(getPerspectiveMatrix()) * homogeneousCoord;
    cameraCoord.z = -1.0; cameraCoord.w = 0.0;
    auto worldCoord = glm::vec3(glm::inverse(getViewMatrix()) * cameraCoord);
    worldCoord = glm::normalize(worldCoord);

    return Ray {transform->getGlobalPosition(), worldCoord};
}

// TODO: cache the perspective and view matrices, to reduce repeating the same computation
glm::mat4 Camera::getPerspectiveMatrix() const {
    return glm::perspective(
            glm::radians(fov),
            ImGui::GetIO().DisplaySize.x / ImGui::GetIO().DisplaySize.y,
            near,
            far
    );
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(
            transform->getGlobalPosition(),
            transform->getGlobalPosition() - transform->getGlobalFrontVec(),
            transform->getGlobalUpVec());
}

glm::vec3 Camera::calcMouseVec(glm::vec2 mousePos) {
    float z;
    float mouseRadiusSq = mousePos.x * mousePos.x + mousePos.y + mousePos.y;
    float radiusSq = radius * radius;
    if (mouseRadiusSq <= 0.5f * radiusSq) {
        z = std::sqrt(radiusSq - mouseRadiusSq);
    }
    else {
        z = 0.5f * radiusSq / std::sqrt(mouseRadiusSq);
    }

    return glm::normalize(glm::vec3(mousePos.x, mousePos.y, z));
}


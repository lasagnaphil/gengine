//
// Created by lasagnaphil on 4/7/18.
//

#ifndef GENGINE_FLYCAMERA_H
#define GENGINE_FLYCAMERA_H

#include "Shader.h"
#include "Ray.h"
#include "Transform.h"
#include "Camera.h"

#include <imgui.h>
#include <SDL2/SDL_events.h>


class FlyCamera : public Camera {
public:
    FlyCamera() = default;
    explicit FlyCamera(Ref<Transform> parent);

    void update(float dt) override;
    void updateCameraVectors() override;
    void renderImGui() override;
    void processInput(SDL_Event& ev) override;

    glm::mat4 getPerspectiveMatrix() const override;
    glm::mat4 getViewMatrix() const override;
    glm::vec3 getPosition() const override {
        return transform->getPosition();
    }
    glm::vec3 getGlobalPosition() const override {
        return transform->getGlobalPosition();
    }

    Ref<Transform> transform;
    // Ref<Transform> trackballFocus;

    float fov = 90.0f;
    float near = 0.1f;
    float far = 1000.0f;
    float movementSpeed = 10.0f;
    float mouseSensitivity = 0.1f;

private:
    float radius = 300.0f;
    float distance = 10.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;
    IntRect viewport;
    bool constrainPitch = true;

    // UI state
    bool enableZoom = false;
};


#endif //GENGINE_FLYCAMERA_H

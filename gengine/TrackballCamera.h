//
// Created by lasagnaphil on 19. 8. 14..
//

#ifndef GENGINE_TRACKBALLCAMERA_H
#define GENGINE_TRACKBALLCAMERA_H

#include "Shader.h"
#include "Ray.h"
#include "Transform.h"
#include "Camera.h"
#include <imgui.h>
#include <SDL2/SDL_events.h>

class TrackballCamera : public Camera {
public:
    TrackballCamera() = default;
    explicit TrackballCamera(Ref<Transform> parent);

    void update(float dt) override;
    void updateCameraVectors() override;
    void renderImGui() override;
    void processInput(SDL_Event& ev) override;

    glm::mat4 getPerspectiveMatrix() const override;
    glm::mat4 getViewMatrix() const override;
    virtual glm::vec3 getPosition() const override {
        return transform->getPosition();
    }
    virtual glm::vec3 getGlobalPosition() const override {
        return transform->getGlobalPosition();
    }

    Ref<Transform> transform;
    Ref<Transform> trackballFocus;

    float fov = 90.0f;
    float near = 0.1f;
    float far = 1000.0f;

    float radius = 300.0f;
    float distance = 10.0f;
    float translationSpeed = 10.0f;
    // float movementSpeed = 10.0f;
    // float mouseSensitivity = 0.1f;

    // debug
    float theta = 0.0f;

private:
    glm::vec3 calcMouseVec(glm::vec2 mousePos);

    IntRect viewport;
    bool constrainPitch = true;

    // UI state
    bool enableZoom = false;
};

#endif //GENGINE_TRACKBALLCAMERA_H

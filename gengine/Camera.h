//
// Created by lasagnaphil on 4/7/18.
//

#ifndef GENGINE_TRACKBALLCAMERA_H
#define GENGINE_TRACKBALLCAMERA_H

#include "Shader.h"
#include "Ray.h"
#include "Transform.h"
#include <imgui.h>
#include <SDL2/SDL_events.h>

struct IntRect {
    int x, y, width, height;
};

class Camera {
public:
    Camera() = default;
    explicit Camera(Ref<Transform> parent);

    void update(float dt);
    void updateCameraVectors();
    void renderImGui();
    void processInput(SDL_Event& ev);

    Ray screenPointToRay(glm::vec2 mousePos) const;
    glm::mat4 getPerspectiveMatrix() const;
    glm::mat4 getViewMatrix() const;


    Ref<Transform> transform;
    // Ref<Transform> trackballFocus;

private:
    glm::vec3 calcMouseVec(glm::vec2 mousePos);

    float radius = 300.0f;
    float distance = 10.0f;
    float translationSpeed = 10.0f;
    float fov = 90.0f;

    float pitch;
    float yaw;
    float movementSpeed;
    float mouseSensitivity;
    IntRect viewport;
    bool constrainPitch;

    // debug
    float theta = 0.0f;

    // UI state
    bool enableZoom = false;
};


#endif //GENGINE_TRACKBALLCAMERA_H

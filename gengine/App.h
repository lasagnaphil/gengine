//
// Created by lasagnaphil on 2/6/18.
//

#ifndef GENGINE_APP_H
#define GENGINE_APP_H

#define GLM_FORCE_RADIANS 1

#include <SDL2/SDL.h>
#include "glad/glad.h"

#include <memory>

#include "Arena.h"
#include "Shader.h"
#include "PhongRenderer.h"
#include "PBRenderer.h"
#include "GizmosRenderer.h"
#include "Camera.h"
#include "DebugRenderer.h"

struct AppSettings {
    enum class Renderer {
        Phong, PBR
    } renderer;

    enum class Camera {
        FlyCamera, TrackballCamera
    } camera;

    bool useDisplayFPS = false;
    int updateFPS = 60;

    static AppSettings defaultPhong() {
        return AppSettings { Renderer::Phong, Camera::FlyCamera, false, 60};
    }
    static AppSettings defaultPBR() {
        return AppSettings { Renderer::PBR, Camera::FlyCamera, false, 60};
    }
};

class App {
public:
    App(AppSettings settings = AppSettings::defaultPhong()) : settings(settings) {}

    virtual ~App();

    void load();
    void startMainLoop();

    virtual void loadResources();
    virtual void processInput(SDL_Event& event);
    virtual void update(float dt);
    virtual void render();
    virtual void release();

protected:
    PhongRenderer phongRenderer;
    PBRenderer pbRenderer;
    DebugRenderer imRenderer;

    AppSettings settings;

    Ref<Transform> rootTransform;
    std::unique_ptr<Camera> camera = nullptr;

private:
    void internalProcessInput();
    void internalUpdate(float dt);
    void internalRender();

    SDL_Window* window;
    SDL_GLContext mainContext;

    bool quit = false;
    float dt;
    int fps;
};

void APIENTRY glDebugOutput(GLenum source,
                            GLenum type,
                            GLuint id,
                            GLenum severity,
                            GLsizei length,
                            const GLchar *message,
                            const void *userParam);

#endif //GENGINE_GAME_H

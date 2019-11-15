//
// Created by lasagnaphil on 2/6/18.
//

#ifndef GENGINE_APP_H
#define GENGINE_APP_H

#define GLM_FORCE_RADIANS 1

#include <SDL2/SDL.h>
#include "glad/glad.h"

#include <memory>

#include "GenAllocator.h"
#include "Shader.h"
#include "PhongRenderer.h"
#include "PBRenderer.h"
#include "GizmosRenderer.h"
#include "Camera.h"

class App {
public:
    App(bool useDisplayFPS = false) :
        useDisplayFPS(useDisplayFPS) {}

    virtual ~App();
    void start();
    virtual void loadResources();
    virtual void processInput(SDL_Event& event);
    virtual void update(float dt);
    virtual void render();
    virtual void release();

    static constexpr Uint32 msPerFrame = 16;

protected:
    PhongRenderer phongRenderer;
    PBRenderer pbRenderer;
    GizmosRenderer gizmosRenderer;
    Ref<Transform> rootTransform;
    std::unique_ptr<Camera> camera = nullptr;

    template <typename T>
    T* initCamera() {
        camera = std::make_unique<T>(rootTransform);
        return dynamic_cast<T*>(camera.get());
    }

private:
    void internalProcessInput();
    void internalLoadResources();
    void internalUpdate(float dt);
    void internalRender();

    SDL_Window* window;
    SDL_GLContext mainContext;

    bool quit = false;
    int updateFPS = 240;
    float dt;
    int fps;
    bool useDisplayFPS = false;
};


#endif //GENGINE_GAME_H

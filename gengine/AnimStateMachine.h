//
// Created by lasagnaphil on 19. 11. 6..
//

#ifndef GENGINE_ANIMSTATEMACHINE_H
#define GENGINE_ANIMSTATEMACHINE_H

#include "glmx/pose.h"
#include "GenAllocator.h"
#include "PoseTree.h"

#include <span.hpp>

struct Animation {
    std::string name;
    std::vector<glmx::pose> poses;
    int fps;
};

struct AnimState {
    std::string name;
    Ref<Animation> animation = {};
    bool loop = false;
};

enum class AnimParamType {
    Bool, Trigger
};

struct AnimParam {
    std::string name;
    AnimParamType type = AnimParamType::Bool;
    union {
        bool value;
    };
};

struct AnimParamCondition {
    std::string name;
    AnimParamType type;
    union {
        bool equals;
    };
};

struct AnimTransition {
    std::string name;
    Ref<AnimState> stateBefore = {};
    Ref<AnimState> stateAfter = {};
    AnimParamCondition condition;
    float transitionTime;
};

struct AnimStateMachine {
private:
    GenAllocator<Animation> anims;
    GenAllocator<AnimState> states;
    GenAllocator<AnimTransition> transitions;
    GenAllocator<AnimParam> params;

    Ref<AnimState> currentState = {};
    Ref<AnimTransition> currentTransition = {};

    bool currentStateEnded = false;

    glmx::pose currentPose;
    glmx::transform offset;

    float stateTime = 0.0f;
    float transitionTime = 0.0f;

public:

    AnimStateMachine() {
        offset.v = glm::vec3(0.f);
        offset.q = glm::identity<glm::quat>();
    }

    template <class T>
    T* get(Ref<T> ref);

    Ref<Animation> addAnimation(const std::string& name, nonstd::span<glmx::pose> poses, int fps=30);
    Ref<AnimState> addState(const std::string& name, Ref<Animation> anim);

    Ref<AnimTransition> addTransition(const std::string& name,
            Ref<AnimState> stateBefore, Ref<AnimState> stateAfter,
            float transitionTime = 0.0f);

    void addCondition(Ref<AnimTransition> transition, const std::string& name, bool value);

    void addTrigger(Ref<AnimTransition> transition, const std::string& name);

    Ref<AnimParam> addParam(const std::string& name, bool value);

    Ref<AnimParam> addParam(const std::string& name);

    Ref<AnimState> getCurrentState() {
        return currentState;
    }

    AnimState* getCurrentStatePtr() {
        return states.get(currentState);
    }

    void setCurrentState(Ref<AnimState> state) {
        currentState = state;
    }

    void setParam(const std::string& name, bool value);

    void setTrigger();

    const glmx::pose& getCurrentPose() const {
        return currentPose;
    }

    void update(float dt);

    void renderImGui(const PoseTree& poseTree);
};

#endif //GENGINE_ANIMSTATEMACHINE_H

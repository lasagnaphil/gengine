//
// Created by lasagnaphil on 19. 11. 6..
//

#ifndef GENGINE_ANIMSTATEMACHINE_H
#define GENGINE_ANIMSTATEMACHINE_H

#include "Pose.h"
#include "GenAllocator.h"

#include <span.hpp>

struct Animation {
    std::string name;
    std::vector<Pose> poses;
    int fps;
};

struct AnimState {
    std::string name;
    Ref<Animation> animation = {};
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

    Pose currentPose;
    float stateTime = 0.0f;
    float transitionTime = 0.0f;

public:
    template <class T>
    T* get(Ref<T> ref);

    Ref<Animation> addAnimation(std::string_view name, nonstd::span<Pose> poses, int fps=30);

    Ref<AnimState> addState(std::string_view name, Ref<Animation> anim);

    Ref<AnimTransition> addTransition(std::string_view name,
            Ref<AnimState> stateBefore, Ref<AnimState> stateAfter,
            float transitionTime = 0.0f);

    void addCondition(Ref<AnimTransition> transition, std::string_view name, bool value);

    void addTrigger(Ref<AnimTransition> transition, std::string_view name);

    Ref<AnimParam> addParam(std::string_view name, bool value);

    Ref<AnimParam> addParam(std::string_view name);

    Ref<AnimState> getCurrentState() {
        return currentState;
    }

    AnimState* getCurrentStatePtr() {
        return states.get(currentState);
    }

    void setCurrentState(Ref<AnimState> state) {
        currentState = state;
    }

    void setParam(std::string_view name, bool value);

    void setTrigger();

    [[nodiscard]] const Pose& getCurrentPose() const {
        return currentPose;
    }

    void update(float dt);
};

#endif //GENGINE_ANIMSTATEMACHINE_H

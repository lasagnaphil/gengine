//
// Created by lasagnaphil on 19. 11. 6..
//

#ifndef GENGINE_ANIMSTATEMACHINE_H
#define GENGINE_ANIMSTATEMACHINE_H

#include "glmx/pose.h"
#include "GenAllocator.h"
#include "PoseTree.h"

#include <span.hpp>
#include <unordered_map>

struct Animation {
    std::string name;
    std::vector<glmx::pose> poses;
    int fps;

    float length() {
        return (poses.size() - 1) * (1.0f / (float)fps);
    }

    glmx::pose getFrame(float time) {
        time = glm::clamp<float>(time, 0, length());
        float dt = 1.0f / (float)fps;
        int f = (int)(time / dt);
        int fp = (f + 1) % poses.size();
        float df = (time - (float)f * dt) / dt;
        return glmx::slerp(poses[f], poses[fp], df);
    }

    glm::vec3 getStartingRootPos() {
        return poses[0].v;
    }

    void setStartingRootPos(glm::vec3 pos) {
        assert(poses.size() > 0);
        glm::vec3 offset = poses[0].v - pos;
        poses[0].v = pos;
        for (int f = 1; f < poses.size(); f++) {
            poses[f].v -= offset;
        }
    }

    void setStartingRootPos(float x, float z) {
        assert(poses.size() > 0);
        glm::vec3 offset = poses[0].v - glm::vec3(x, 0, z);
        offset.y = 0;
        poses[0].v.x = x;
        poses[0].v.z = z;
        for (int f = 1; f < poses.size(); f++) {
            poses[f].v -= offset;
        }
    }
};

struct AnimState {
    std::string name;
    Ref<Animation> animation = {};
};

enum class AnimParamType {
    None, Bool, Trigger
};

struct AnimParam {
    std::string name;
    AnimParamType type = AnimParamType::None;
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
    float stateBeforeTime;
    float stateAfterTime;
    float transitionTime;
};

struct AnimStateMachine {
private:
    GenAllocator<Animation> anims;
    GenAllocator<AnimState> states;
    GenAllocator<AnimTransition> transitions;
    std::unordered_map<std::string, AnimParam> params;

    Ref<AnimState> currentState = {};
    Ref<AnimTransition> currentTransition = {};

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
            float stateBeforeTime, float stateAfterTime, float transitionTime);

    void setTransitionCondition(Ref<AnimTransition> transition, const std::string& name, bool value);

    void setTransitionTrigger(Ref<AnimTransition> transition, const std::string& name);

    AnimParam& addParam(const std::string& name, bool value);

    AnimParam& addParam(const std::string& name);

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

    void setTrigger(const std::string& name);

    const glmx::pose& getCurrentPose() const {
        return currentPose;
    }

    void update(float dt);

    void renderImGui(const PoseTree& poseTree);

private:

    std::tuple<Ref<AnimTransition>, float> selectNextTransition();

    void stateUpdate(float dt);
    void transitionUpdate(float dt);


};

#endif //GENGINE_ANIMSTATEMACHINE_H

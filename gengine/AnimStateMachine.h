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
    float rotation = 0.0f;

    float length() {
        assert(poses.size() > 0);

        return (poses.size() - 1) * (1.0f / (float)fps);
    }

    glmx::pose getFrame(float time) {
        assert(poses.size() > 0);

        time = glm::clamp<float>(time, 0, length());
        float dt = 1.0f / (float)fps;
        int f = (int)(time / dt);
        int fp = (f + 1) % poses.size();
        float df = (time - (float)f * dt) / dt;
        return glmx::slerp(poses[f], poses[fp], df);
    }

    glm::vec3 getStartingRootPos() {
        assert(poses.size() > 0);

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

    void setStartingRootTrans(float x, float z, float rot, float rotThreshold = 0.0f) {
        assert(poses.size() > 0);

        float originalRot = glmx::extractYRot(poses[0].q[0]);

        glmx::transform offset;
        offset.v.x = x - poses[0].v.x;
        offset.v.z = z - poses[0].v.z;
        if (rot - originalRot < rotThreshold) {
            offset.q = glm::identity<glm::quat>();
        }
        else {
            offset.q = glm::angleAxis(rot - originalRot, glm::vec3(0, 1, 0));
        }

        for (int f = 1; f < poses.size(); f++) {
            glm::vec3 dv = poses[f].v - poses[0].v;
            dv.y = 0;
            dv = offset.q * dv;
            poses[f].v.x = poses[0].v.x + dv.x + offset.v.x;
            poses[f].v.z = poses[0].v.z + dv.z + offset.v.z;
            poses[f].q[0] = offset.q * poses[f].q[0];
        }

        poses[0].v.x = x;
        poses[0].v.z = z;
        poses[0].q[0] = offset.q * poses[0].q[0];

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

    Animation currentAnim;
    Animation blendAnim1;
    Animation blendAnim2;

    float stateTime = 0.0f;
    float transitionTime = 0.0f;


public:

    // DEBUG
    glmx::pose p1, p2;

    AnimStateMachine() {
        // anims.expand(32);
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
        currentAnim = *anims.get(states.get(state)->animation);
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

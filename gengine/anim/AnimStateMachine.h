//
// Created by lasagnaphil on 19. 11. 6..
//

#ifndef GENGINE_ANIMSTATEMACHINE_H
#define GENGINE_ANIMSTATEMACHINE_H

#include "glmx/pose.h"
#include "Arena.h"
#include "PoseTree.h"

#include <span.hpp>
#include <unordered_map>

#include "anim/MotionClip.h"

struct Animation {
    std::string name;
    MotionClip clip;
    int fps;

    float length() {
        assert(clip.numFrames > 0);

        return (clip.numFrames - 1) * (1.0f / (float)fps);
    }

    glmx::pose getFrame(float time) {
        assert(clip.numFrames > 0);

        time = glm::clamp<float>(time, 0, length());
        float dt = 1.0f / (float)fps;
        int f = (int)(time / dt);
        int fp = (f + 1) % clip.numFrames;
        float df = (time - (float)f * dt) / dt;
        glmx::pose newPose = glmx::pose::empty(clip.getFrame(f).size());
        glmx::slerp(clip.getFrame(f), clip.getFrame(fp), df, newPose.getView());
        return newPose;
    }

    glm::vec3 getStartingRootPos() {
        assert(clip.numFrames > 0);

        return clip.rootPos(0);
    }

    void setStartingRootPos(glm::vec3 pos) {
        assert(clip.numFrames > 0);

        glm::vec3 offset = clip.rootPos(0) - pos;
        clip.rootPos(0) = pos;
        for (int f = 1; f < clip.numFrames; f++) {
            clip.rootPos(f) -= offset;
        }
    }

    void setStartingRootPos(float x, float z) {
        assert(clip.numFrames > 0);

        glm::vec3 offset = clip.rootPos(0) - glm::vec3(x, 0, z);
        offset.y = 0;
        clip.rootPos(0).x = x;
        clip.rootPos(0).z = z;
        for (int f = 1; f < clip.numFrames; f++) {
            clip.rootPos(f) -= offset;
        }
    }

    void setStartingRootTrans(float x, float z, float rot, float rotThreshold = 0.0f) {
        assert(clip.numFrames > 0);
        glmx::pose_view startPose = clip.getFrame(0);

        float originalRot = glmx::extractYRot(startPose.q(0));

        glmx::transform offset;
        offset.v.x = x - startPose.v().x;
        offset.v.z = z - startPose.v().z;
        if (glm::abs(rot - originalRot) < rotThreshold) {
            offset.q = glm::identity<glm::quat>();
        }
        else {
            offset.q = glm::angleAxis(rot - originalRot, glm::vec3(0, 1, 0));
        }

        for (int f = 1; f < clip.numFrames; f++) {
            glmx::pose_view curPose = clip.getFrame(f);
            glm::vec3 dv = curPose.v() - startPose.v();
            dv.y = 0;
            dv = offset.q * dv;
            curPose.v().x = startPose.v().x + dv.x + offset.v.x;
            curPose.v().z = startPose.v().z + dv.z + offset.v.z;
            curPose.q(0) = offset.q * curPose.q(0);
        }

        startPose.v().x = x;
        startPose.v().z = z;
        startPose.q(0) = offset.q * startPose.q(0);
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
    Arena<Animation> anims;
    Arena<AnimState> states;
    Arena<AnimTransition> transitions;
    std::unordered_map<std::string, AnimParam> params;

    Ref<AnimState> currentState = {};
    Ref<AnimTransition> currentTransition = {};

    glmx::pose currentPose;

    Animation currentAnim;
    Animation blendAnim1;
    Animation blendAnim2;

    float stateTime = 0.0f;
    float transitionTime = 0.0f;

    bool rotationEnabled = true;

public:

    // DEBUG
    glmx::pose p1, p2;

    AnimStateMachine() {
        // anims.expand(32);
    }

    template <class T>
    T* get(Ref<T> ref);

    Ref<Animation> addAnimation(const std::string& name, MotionClipView clip, int fps=30);
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
        currentTransition = {};
        currentAnim = *anims.get(states.get(state)->animation);
        currentPose = currentAnim.getFrame(0);
        stateTime = 0.0f;
        transitionTime = 0.0f;
    }

    float getStateTime() { return stateTime; }
    float getTransitionTime() { return transitionTime; }

    Animation& getCurrentAnim() { return currentAnim; }
    const Animation& getCurrentAnim() const { return currentAnim; }

    void moveCurrentAnim(glm::vec3 offset) {
        for (int f = 0; f < currentAnim.clip.numFrames; f++) {
            currentAnim.clip.rootPos(f) += offset;
        }
    }

    void enableRotation(bool enable) {
        rotationEnabled = enable;
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

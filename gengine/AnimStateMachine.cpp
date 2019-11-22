//
// Created by lasagnaphil on 19. 11. 6..
//

#include <glmx/transform.h>
#include "AnimStateMachine.h"
#include <imgui.h>
#include <iostream>

Ref<Animation> AnimStateMachine::addAnimation(const std::string& name, nonstd::span<glmx::pose> poses, int fps) {
    auto animRef = anims.make();
    auto anim = anims.get(animRef);
    anim->name = name;
    anim->poses = std::vector(poses.data(), poses.data() + poses.size());
    anim->fps = fps;
    return animRef;
}

Ref<AnimState> AnimStateMachine::addState(const std::string& name, Ref<Animation> anim) {
    auto stateRef = states.make();
    auto state = states.get(stateRef);
    state->name = name;
    state->animation = anim;
    return stateRef;
}

Ref<AnimTransition>
AnimStateMachine::addTransition(const std::string& name, Ref<AnimState> stateBefore, Ref<AnimState> stateAfter,
                                float stateBeforeTime, float stateAfterTime, float transitionTime) {

    auto transitionRef = transitions.make();
    auto transition = transitions.get(transitionRef);
    transition->name = name;
    transition->stateBefore = stateBefore;
    transition->stateAfter = stateAfter;
    transition->stateBeforeTime = stateBeforeTime;
    transition->stateAfterTime = stateAfterTime;
    transition->condition = {"", AnimParamType::None};
    transition->transitionTime = transitionTime;
    return transitionRef;
}

void AnimStateMachine::setTransitionCondition(Ref<AnimTransition> transition, const std::string& name, bool value) {
    auto trans = transitions.get(transition);
    trans->condition.name = name;
    trans->condition.type = AnimParamType::Bool;
    trans->condition.equals = value;
}

void AnimStateMachine::setTransitionTrigger(Ref<AnimTransition> transition, const std::string& name) {
    auto trans = transitions.get(transition);
    trans->condition.name = name;
    trans->condition.type = AnimParamType::Trigger;
}

AnimParam& AnimStateMachine::addParam(const std::string& name, bool value) {
    params[name] = {name, AnimParamType::Bool};
    params[name].value = value;
    return params[name];
}

AnimParam& AnimStateMachine::addParam(const std::string& name) {
    params[name] = {name, AnimParamType::Trigger};
    params[name].value = false;
    return params[name];
}

void AnimStateMachine::setParam(const std::string& name, bool value) {
    transitions.forEach([&](AnimTransition& trans, Ref<AnimTransition> ref) {
        if (currentState == trans.stateBefore &&
            trans.condition.name == name && trans.condition.type == AnimParamType::Bool) {
            params[trans.condition.name].value = value;
        }
    });
}

void AnimStateMachine::setTrigger(const std::string& name) {
    transitions.forEach([&](AnimTransition& trans, Ref<AnimTransition> ref) {
        if (currentState == trans.stateBefore &&
            trans.condition.name == name && trans.condition.type == AnimParamType::Trigger) {
            params[trans.condition.name].value = true;
        }
    });
}

std::tuple<Ref<AnimTransition>, float> AnimStateMachine::selectNextTransition() {
    Ref<AnimTransition> nextTrans = {};
    Ref<AnimTransition> noCondTrans = {};

    float timeFromTransStart = 0.0f;

    transitions.forEachUntil([&](AnimTransition& trans, Ref<AnimTransition> transRef) {
        if (currentState == trans.stateBefore) {
            auto& curState = *states.get(currentState);
            auto& curAnim = *anims.get(curState.animation);
            if (trans.condition.type == AnimParamType::None && !noCondTrans) {
                if (stateTime >= curAnim.length() - trans.stateBeforeTime) {
                    noCondTrans = transRef;
                    timeFromTransStart = stateTime - curAnim.length() + trans.stateBeforeTime;
                }
            }
            else if (trans.condition.type == AnimParamType::Bool) {
                bool triggered = trans.condition.equals == params[trans.condition.name].value;
                if (triggered) {
                    nextTrans = transRef;
                    return true;
                }
            }
            else if (trans.condition.type == AnimParamType::Trigger) {
                bool triggered = params[trans.condition.name].value;
                if (triggered) {
                    nextTrans = transRef;
                    return true;
                }
            }
        }
        return false;
    });

    if (nextTrans) {
        return {nextTrans, timeFromTransStart};
    } else{
        return {noCondTrans, timeFromTransStart};
    }
}

void AnimStateMachine::stateUpdate(float dt) {
    stateTime += dt;
    auto& state = *states.get(currentState);
    auto& anim = *anims.get(state.animation);

    float frameDt = 1.0f / (float)anim.fps;
    int animSize = anim.poses.size();
    float animTime = animSize * frameDt;

    auto [nextTransRef, timeSinceTransStart] = selectNextTransition();
    if (nextTransRef) {
        auto& nextTrans = *transitions.get(nextTransRef);

        currentState = {};
        currentTransition = nextTransRef;
        transitionTime = 0.0f;

        transitionUpdate(timeSinceTransStart);

        return;
    }

    currentPose = anim.getFrame(stateTime);
}

void AnimStateMachine::transitionUpdate(float dt) {
    auto& curTrans = *transitions.get(currentTransition);
    auto& stateBefore = *states.get(curTrans.stateBefore);
    auto& stateAfter = *states.get(curTrans.stateAfter);
    auto& stateBeforeAnim = *anims.get(stateBefore.animation);
    auto& stateAfterAnim = *anims.get(stateAfter.animation);

    transitionTime += dt;

    if (transitionTime > curTrans.transitionTime) {
        float p2_start_time = curTrans.stateAfterTime + (transitionTime - curTrans.transitionTime);
        glm::vec3 p1_start_pos = stateBeforeAnim.getFrame(stateTime).v;
        glm::vec3 p2_start_pos = stateAfterAnim.getFrame(p2_start_time).v;
        glm::vec3 last_pos = stateAfterAnim.getFrame(curTrans.transitionTime).v;
        offset.v += last_pos + p1_start_pos - p2_start_pos;
        offset.v.y = 0.0f;

        currentState = curTrans.stateAfter;
        currentTransition = {};
        stateTime = curTrans.stateAfterTime;

        stateUpdate(transitionTime - curTrans.transitionTime);

        return;
    }

    // TODO: implement time rescaling
    assert(curTrans.stateBeforeTime == curTrans.stateAfterTime);
    assert(stateBeforeAnim.fps == stateAfterAnim.fps);

    auto easeFunc = [](float t) { return -0.5 * cos(M_PI * t) + 0.5; };

    float u = easeFunc(transitionTime / curTrans.transitionTime);
    glm::vec3 p1_start_pos = stateBeforeAnim.getFrame(stateTime).v;
    glm::vec3 p2_start_pos = stateAfterAnim.getFrame(0).v;
    glmx::pose p1 = stateBeforeAnim.getFrame(stateTime + transitionTime);
    glmx::pose p2 = stateAfterAnim.getFrame(transitionTime);
    p2.v.x = p2.v.x - p2_start_pos.x + p1_start_pos.x;
    p2.v.z = p2.v.z - p2_start_pos.z + p1_start_pos.z;
    currentPose = glmx::slerp(p1, p2, u);
}

void AnimStateMachine::update(float dt) {
    if (currentState) {
        stateUpdate(dt);
    }
    else if (currentTransition) {
        transitionUpdate(dt);
    }

    glmx::transform_root(currentPose, offset);

    // Now clear all the activated triggers
    for (auto& [name, param] : params) {
        if (param.type == AnimParamType::Trigger) {
            param.value = false;
        }
    }
}

void AnimStateMachine::renderImGui(const PoseTree& poseTree) {
    ImGui::Begin("Anim State Machine");

    if (currentState) {
        AnimState& curState = *states.get(currentState);
        Animation& curAnimation = *anims.get(curState.animation);
        ImGui::Text("Current State: %s", curState.name.c_str());
        ImGui::SliderFloat("State Time", &stateTime, 0, curAnimation.length());
    }
    else if (currentTransition) {
        auto& curTrans = *transitions.get(currentTransition);
        auto& stateBefore = *states.get(curTrans.stateBefore);
        auto& stateAfter = *states.get(curTrans.stateAfter);
        auto& stateBeforeAnim = *anims.get(stateBefore.animation);
        auto& stateAfterAnim = *anims.get(stateAfter.animation);

        ImGui::Text("Current Transition: %s (%s -> %s)",
                curTrans.name.c_str(), stateBefore.name.c_str(), stateAfter.name.c_str());
        ImGui::SliderFloat("Transition Time", &transitionTime, 0, curTrans.stateBeforeTime + curTrans.stateAfterTime);
    }

    ImGui::InputFloat3("Offset translation", (float*)&offset.v);
    glm::vec3 v = glmx::quatToEuler(offset.q, EulOrdZYXs);
    ImGui::SliderFloat3("Offset rotation", (float*)&v, -M_PI, M_PI);
    offset.q = glmx::eulerToQuat(v, EulOrdZYXs);

    ImGui::InputFloat3(poseTree[0].name.c_str(), (float*)&currentPose.v);
    for (uint32_t i = 0; i < currentPose.size(); i++) {
        auto& node = poseTree[i];
        glm::vec3 v = glmx::quatToEuler(currentPose.q[i], EulOrdZYXs);
        ImGui::SliderFloat3(node.name.c_str(), (float*)&v, -M_PI, M_PI);
        currentPose.q[i] = glmx::eulerToQuat(v, EulOrdZYXs);
    }

    ImGui::End();
}

#define GETTER(__Type, __storage) \
template <> __Type* AnimStateMachine::get(Ref<__Type> ref) { return __storage.get(ref); }

GETTER(Animation, anims)
GETTER(AnimState, states)
GETTER(AnimTransition, transitions)

#undef GETTER


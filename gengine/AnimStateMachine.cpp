//
// Created by lasagnaphil on 19. 11. 6..
//

#include <glmx/transform.h>
#include "AnimStateMachine.h"
#include <imgui.h>

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
                                float transitionTime) {

    auto transitionRef = transitions.make();
    auto transition = transitions.get(transitionRef);
    transition->name = name;
    transition->stateBefore = stateBefore;
    transition->stateAfter = stateAfter;
    transition->transitionTime = transitionTime;
    return transitionRef;
}

void AnimStateMachine::addCondition(Ref<AnimTransition> transition, const std::string& name, bool value) {
    auto trans = transitions.get(transition);
    trans->condition.name = name;
    trans->condition.type = AnimParamType::Bool;
    trans->condition.equals = value;
}

void AnimStateMachine::addTrigger(Ref<AnimTransition> transition, const std::string& name) {
    auto trans = transitions.get(transition);
    trans->condition.name = name;
    trans->condition.type = AnimParamType::Bool;
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

void AnimStateMachine::setTrigger() {
    transitions.forEach([&](AnimTransition& trans, Ref<AnimTransition> ref) {
        if (currentState == trans.stateBefore && trans.condition.type == AnimParamType::Trigger) {
            params[trans.condition.name].value = true;
        }
    });
}

Ref<AnimTransition> AnimStateMachine::selectNextTransition() {
    Ref<AnimTransition> nextTrans = {};
    Ref<AnimTransition> noCondTrans = {};

    transitions.forEachUntil([&](AnimTransition& trans, Ref<AnimTransition> transRef) {
        if (currentState == trans.stateBefore) {
            if (trans.condition.type == AnimParamType::None && noCondTrans) {
                noCondTrans = transRef;
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
    if (nextTrans.isNull()) {
        nextTrans = noCondTrans;
    }

    return nextTrans;
}

void AnimStateMachine::update(float dt) {
    if (currentState && !currentStateEnded) {
        stateTime += dt;
        auto& state = *states.get(currentState);
        auto& anim = *anims.get(state.animation);

        float frameDt = 1.0f / (float)anim.fps;
        int animSize = anim.poses.size();
        float animTime = animSize * frameDt;

        auto nextTransRef = selectNextTransition();
        if (nextTransRef) {
            auto& nextTrans = *transitions.get(nextTransRef);
            if (nextTrans.condition.type != AnimParamType::None) {
                currentState = {};
                currentStateEnded = false;
                currentTransition = nextTransRef;

                int frameNum = (int)(stateTime / frameDt);
                glmx::pose& pBegin = anim.poses[0];
                glmx::pose& pEnd = anim.poses[frameNum % animSize];

                auto& stateAfter = *states.get(nextTrans.stateAfter);
                auto& stateAfterAnim = *anims.get(stateAfter.animation);
                glmx::pose nextPose = stateAfterAnim.poses[0];

                offset.v += (pEnd.v - pBegin.v);

                return;
            }
        }

        if (stateTime > animTime) {
            if (states.get(currentState)->loop) {
                glmx::pose& pBegin = anim.poses[0];
                glmx::pose& pEnd = anim.poses[anim.poses.size() - 1];
                glmx::transform t {};
                t.v = pEnd.v - pBegin.v;
                // t.q = glm::conjugate(pBegin.q[0]) * pEnd.q[0];
                offset = offset * t;

                stateTime -= animTime;
            }
            else {
                AnimState& curState = *states.get(currentState);
                Animation& curAnim = *anims.get(curState.animation);

                currentTransition = selectNextTransition();

                if (currentTransition) {
                    currentState = {};
                    currentStateEnded = false;
                    return;
                }
                else {
                    stateTime = animTime;
                    currentStateEnded = true;
                    return;
                }
            }
        }

        int frameNum = (int)(stateTime / frameDt);
        float interpFactor = stateTime / frameDt - (float)frameNum;

        glmx::pose& p1 = anim.poses[frameNum % animSize];
        glmx::pose& p2 = anim.poses[(frameNum + 1) % animSize];


        if (frameNum == animSize - 1)
            currentPose = p1;
        else
            currentPose = slerp(p1, p2, interpFactor);

        glmx::transform_root(currentPose, offset);
    }
    else if (currentTransition) {
        auto& curTrans = *transitions.get(currentTransition);
        auto& stateBefore = *states.get(curTrans.stateBefore);
        auto& stateAfter = *states.get(curTrans.stateAfter);
        auto& stateBeforeAnim = *anims.get(stateBefore.animation);
        auto& stateAfterAnim = *anims.get(stateAfter.animation);

        transitionTime += dt;

        if (transitionTime > curTrans.transitionTime) {
            currentState = curTrans.stateAfter;
            currentTransition = {};
            stateTime = 0.0f;
            transitionTime = 0.0f;
            return;
        }

        // TODO: blend between poses
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
        ImGui::SliderFloat("Transition Time", &transitionTime, 0, curTrans.transitionTime);
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


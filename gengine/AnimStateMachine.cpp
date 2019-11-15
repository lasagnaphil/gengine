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

Ref<AnimParam> AnimStateMachine::addParam(const std::string& name, bool value) {
    auto paramRef = params.make();
    auto param = params.get(paramRef);
    param->name = name;
    param->type = AnimParamType::Bool;
    param->value = value;
    return paramRef;
}

Ref<AnimParam> AnimStateMachine::addParam(const std::string& name) {
    auto paramRef = params.make();
    auto param = params.get(paramRef);
    param->name = name;
    return paramRef;
}

void AnimStateMachine::setParam(const std::string& name, bool value) {
    transitions.forEach([&](AnimTransition& trans, Ref<AnimTransition> ref) {
        if (currentState == trans.stateBefore &&
            trans.condition.name == name && trans.condition.type == AnimParamType::Bool) {
            currentState = {};
            currentTransition = ref;
            params.forEach([&](AnimParam& param, Ref<AnimParam> ref) {
                if (param.name == trans.condition.name) {
                    param.value = value;
                }
            });
        }
    });
}

void AnimStateMachine::setTrigger() {
    transitions.forEach([&](AnimTransition& trans, Ref<AnimTransition> ref) {
        if (currentState == trans.stateBefore && trans.condition.type == AnimParamType::Trigger) {
            currentState = {};
            currentTransition = ref;
        }
    });
}

void AnimStateMachine::update(float dt) {
    if (currentState && !currentStateEnded) {
        stateTime += dt;
        auto statePtr = states.get(currentState);
        auto animPtr = anims.get(statePtr->animation);

        float frameDt = 1.0f / (float)animPtr->fps;
        int animSize = animPtr->poses.size();
        float animTime = animSize * frameDt;

        if (stateTime > animTime) {
            if (states.get(currentState)->loop) {
                glmx::transform t {};
                glmx::pose& pBegin = animPtr->poses[0];
                glmx::pose& pEnd = animPtr->poses[animPtr->poses.size() - 1];
                t.v = pEnd.v - pBegin.v;
                // t.q = glm::conjugate(pBegin.q[0]) * pEnd.q[0];
                offset = offset * t;

                stateTime -= animTime;
            }
            else {
                stateTime = animTime;
                currentStateEnded = true;
                return;
            }
        }

        int frameNum = (int)(stateTime / frameDt);
        float interpFactor = stateTime / frameDt - (float)frameNum;

        glmx::pose& p1 = animPtr->poses[frameNum % animSize];
        glmx::pose& p2 = animPtr->poses[(frameNum + 1) % animSize];


        if (frameNum == animSize - 1)
            currentPose = p1;
        else
            currentPose = slerp(p1, p2, interpFactor);

        glmx::transform_root(currentPose, offset);
    }
    else if (currentTransition) {
        transitionTime += dt;

        // TODO: blend between poses
    }
}

void AnimStateMachine::renderImGui(const PoseTree& poseTree) {
    ImGui::Begin("Anim State Machine");

    ImGui::InputFloat3("Offset translation", (float*)&offset.v);
    glm::vec3 v = glmx::quatToEuler(offset.q, EulOrdZYXs);
    ImGui::SliderFloat3("Offset rotation", (float*)&v, -M_PI, M_PI);
    offset.q = glmx::eulerToQuat(v, EulOrdZYXs);

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
GETTER(AnimParam, params)

#undef GETTER


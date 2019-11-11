//
// Created by lasagnaphil on 19. 11. 6..
//

#include "AnimStateMachine.h"

Ref<Animation> AnimStateMachine::addAnimation(std::string_view name, nonstd::span<Pose> poses, int fps) {
    auto animRef = anims.make();
    auto anim = anims.get(animRef);
    anim->name = name;
    anim->poses = std::vector(poses.data(), poses.data() + poses.size());
    anim->fps = fps;
    return animRef;
}

Ref<AnimState> AnimStateMachine::addState(std::string_view name, Ref<Animation> anim) {
    auto stateRef = states.make();
    auto state = states.get(stateRef);
    state->name = name;
    state->animation = anim;
    return stateRef;
}

Ref<AnimTransition>
AnimStateMachine::addTransition(std::string_view name, Ref<AnimState> stateBefore, Ref<AnimState> stateAfter,
                                float transitionTime) {

    auto transitionRef = transitions.make();
    auto transition = transitions.get(transitionRef);
    transition->name = name;
    transition->stateBefore = stateBefore;
    transition->stateAfter = stateAfter;
    transition->transitionTime = transitionTime;
    return transitionRef;
}

void AnimStateMachine::addCondition(Ref<AnimTransition> transition, std::string_view name, bool value) {
    auto trans = transitions.get(transition);
    trans->condition.name = name;
    trans->condition.type = AnimParamType::Bool;
    trans->condition.equals = value;
}

void AnimStateMachine::addTrigger(Ref<AnimTransition> transition, std::string_view name) {
    auto trans = transitions.get(transition);
    trans->condition.name = name;
    trans->condition.type = AnimParamType::Bool;
}

Ref<AnimParam> AnimStateMachine::addParam(std::string_view name, bool value) {
    auto paramRef = params.make();
    auto param = params.get(paramRef);
    param->name = name;
    param->type = AnimParamType::Bool;
    param->value = value;
    return paramRef;
}

Ref<AnimParam> AnimStateMachine::addParam(std::string_view name) {
    auto paramRef = params.make();
    auto param = params.get(paramRef);
    param->name = name;
    return paramRef;
}

void AnimStateMachine::setParam(std::string_view name, bool value) {
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
    if (currentState) {
        stateTime += dt;
        auto statePtr = states.get(currentState);
        auto animPtr = anims.get(statePtr->animation);

        float frameDt = 1.0f / (float)animPtr->fps;
        int animSize = animPtr->poses.size();
        float animTime = animSize * frameDt;
        if (stateTime > animTime) {
            stateTime -= animTime;
        }

        int frameNum = (int)(stateTime / frameDt);
        float interpFactor = stateTime / frameDt - (float)frameNum;
        Pose& p1 = animPtr->poses[frameNum % animSize];
        Pose& p2 = animPtr->poses[(frameNum + 1) % animSize];

        // currentPose = p1 * exp(interpFactor * log(p2 / p1));
        // currentPose = slerp(p1, p2, interpFactor);
        currentPose = p1;
    }
    else if (currentTransition) {
        transitionTime += dt;

        // TODO: blend between poses
    }
}

#define GETTER(__Type, __storage) \
template <> __Type* AnimStateMachine::get(Ref<__Type> ref) { return __storage.get(ref); }

GETTER(Animation, anims)
GETTER(AnimState, states)
GETTER(AnimTransition, transitions)
GETTER(AnimParam, params)

#undef GETTER


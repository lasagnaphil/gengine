//
// Created by lasagnaphil on 19. 11. 6..
//

#ifndef GENGINE_ANIMSTATEMACHINE_H
#define GENGINE_ANIMSTATEMACHINE_H

#include "GenAllocator.h"

struct Animation {
    std::string name;
    std::vector<Pose> poses;
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
    GenAllocator<Animation> anims;
    GenAllocator<AnimState> states;
    GenAllocator<AnimTransition> transitions;
    GenAllocator<AnimParam> params;

    Ref<AnimState> currentState;
    Ref<AnimTransition> currentTransition;

    Ref<Animation> addAnimation(std::string_view name, nonstd::span<Pose> poses) {
        auto animRef = anims.make();
        auto anim = anims.get(animRef);
        anim->name = name;
        anim->poses = std::vector(poses.data(), poses.data() + poses.size());
        return animRef;
    }

    Ref<AnimState> addState(std::string_view name, Ref<Animation> anim) {
        auto stateRef = states.make();
        auto state = states.get(stateRef);
        state->name = name;
        state->animation = anim;
        return stateRef;
    }

    Ref<AnimTransition> addTransition(std::string_view name,
            Ref<AnimState> stateBefore, Ref<AnimState> stateAfter,
            float transitionTime = 0.0f) {

        auto transitionRef = transitions.make();
        auto transition = transitions.get(transitionRef);
        transition->name = name;
        transition->stateBefore = stateBefore;
        transition->stateAfter = stateAfter;
        transition->transitionTime = transitionTime;
        return transitionRef;
    }

    void addCondition(Ref<AnimTransition> transition, std::string_view name, bool value) {
        auto trans = transitions.get(transition);
        trans->condition.name = name;
        trans->condition.type = AnimParamType::Bool;
        trans->condition.equals = value;
    }

    void addTrigger(Ref<AnimTransition> transition, std::string_view name) {
        auto trans = transitions.get(transition);
        trans->condition.name = name;
        trans->condition.type = AnimParamType::Bool;
    }

    Ref<AnimParam> addParam(std::string_view name, bool value) {
        auto paramRef = params.make();
        auto param = params.get(paramRef);
        param->name = name;
        param->type = AnimParamType::Bool;
        param->value = value;
    }

    Ref<AnimParam> addParam(std::string_view name) {
        auto paramRef = params.make();
        auto param = params.get(paramRef);
        param->name = name;
    }

    void setParam(bool value) {
        transitions.forEach([&](AnimTransition& trans, Ref<AnimTransition> ref) {
            if (currentState == trans.stateBefore && trans.condition.type == AnimParamType::Bool) {
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

    void setTrigger() {
        transitions.forEach([&](AnimTransition& trans, Ref<AnimTransition> ref) {
            if (currentState == trans.stateBefore && trans.condition.type == AnimParamType::Trigger) {
                currentState = {};
                currentTransition = ref;
            }
        });
    }
};

#endif //GENGINE_ANIMSTATEMACHINE_H

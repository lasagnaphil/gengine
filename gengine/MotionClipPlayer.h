//
// Created by lasagnaphil on 19. 5. 9.
//

#ifndef PHYSICS_BENCHMARKS_MOTIONCLIPPLAYER_H
#define PHYSICS_BENCHMARKS_MOTIONCLIPPLAYER_H

#include "MotionClipData.h"
#include "LineMesh.h"

struct GizmosRenderer;

struct MotionClipPlayer {
protected:
    int currentFrameIdx = 0;
    bool isPlaying = false;

public:
    virtual uint32_t getNumFrames() = 0;
    virtual float getFrameTime() = 0;

    bool playing() {
        return isPlaying;
    }

    void play() {
        isPlaying = true;
    }

    void pause() {
        isPlaying = false;
    }

    void stop() {
        isPlaying = false;
        currentFrameIdx = 0;
    }

    void previousFrame() {
        currentFrameIdx--;
        if (currentFrameIdx == -1) {
            currentFrameIdx = getNumFrames() - 1;
        }
    }

    void nextFrame() {
        currentFrameIdx++;
        if (currentFrameIdx >= getNumFrames()) {
            currentFrameIdx = 0;
        }
    }

    uint32_t getFrameIdx() {
        return currentFrameIdx;
    }

    void setFrame(uint32_t frameIdx) {
        assert(frameIdx >= 0 && frameIdx < getNumFrames());
        currentFrameIdx = frameIdx;
    }

    void update(float dt);

    virtual void renderImGui();
};

struct BVHMotionClipPlayer : public MotionClipPlayer {
    MotionClipData* clip;

    Ref<LineMesh> trajectoryMesh;
    Ref<LineMaterial> trajectoryMat;

    BVHMotionClipPlayer(MotionClipData* clip = nullptr) : MotionClipPlayer(), clip(clip) {}

    void setClip(MotionClipData* clip) {
        this->clip = clip;
    }

    uint32_t getNumFrames() override {
        assert(clip);
        return clip->numFrames;
    }

    float getFrameTime() override {
        assert(clip);
        return clip->frameTime;
    }

    const glmx::pose& getPoseState() const {
        assert(clip);
        assert(currentFrameIdx < clip->numFrames);
        return clip->getFrameState(currentFrameIdx);
    }

    glmx::pose& getPoseState() {
        assert(clip);
        assert(currentFrameIdx < clip->numFrames);
        return clip->getFrameState(currentFrameIdx);
    }

    void renderImGui() override;
};

#endif //PHYSICS_BENCHMARKS_MOTIONCLIPPLAYER_H

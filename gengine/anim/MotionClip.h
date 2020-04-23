//
// Created by lasagnaphil on 20. 4. 22..
//

#ifndef DEEPMIMIC_MOTIONCLIP_H
#define DEEPMIMIC_MOTIONCLIP_H

struct MotionClipView {
    float* data = nullptr;
    uint32_t numChannels = 0;
    uint32_t numFrames = 0;

    MotionClipView() = default;

    glmx::pose_view getFrame(uint32_t frameIdx) {
        assert(frameIdx >= 0);
        assert(frameIdx < numFrames);
        float* v_ptr = data + frameIdx * numChannels;
        float* q_ptr = v_ptr + 3;
        return glmx::pose_view(v_ptr, q_ptr, (numChannels-3)/4);
    }

    void setFrame(uint32_t frameIdx, glmx::pose_view pose) {
        assert(frameIdx >= 0);
        assert(frameIdx < numFrames);
        float* v_ptr = data + frameIdx * numChannels;
        float* q_ptr = v_ptr + 3;
        std::memcpy(v_ptr, pose.v_ptr, 3*sizeof(float));
        std::memcpy(q_ptr, pose.q_ptr, 4*pose.size*sizeof(float));
    }

    glm::vec3& rootPos(uint32_t frameIdx) {
        assert(frameIdx >= 0);
        assert(frameIdx < numFrames);
        float* v_ptr = data + frameIdx * numChannels;
        return *((glm::vec3*)v_ptr);
    }

    const glm::vec3& rootPos(uint32_t frameIdx) const {
        assert(frameIdx >= 0);
        assert(frameIdx < numFrames);
        const float* v_ptr = data + frameIdx * numChannels;
        return *((const glm::vec3*)v_ptr);
    }

    glm::quat& jointRot(uint32_t frameIdx, uint32_t jointIdx) {
        assert(frameIdx >= 0);
        assert(frameIdx < numFrames);
        assert(jointIdx >= 0);
        assert(jointIdx < (numChannels-3)/4);
        float* q_ptr = data + frameIdx * numChannels + 3 + 4*jointIdx;
        return *((glm::quat*)q_ptr);
    }

    const glm::quat& jointRot(uint32_t frameIdx, uint32_t jointIdx) const {
        assert(frameIdx > 0);
        assert(frameIdx < numFrames);
        assert(jointIdx >= 0);
        assert(jointIdx < (numChannels-3)/4);
        const float* q_ptr = data + frameIdx * numChannels + 3 + 4*jointIdx;
        return *((const glm::quat*)q_ptr);
    };

    MotionClipView slice(uint32_t start, uint32_t end) {
        MotionClipView view;
        assert(start >= 0);
        assert(end <= numFrames);
        assert(start < end);
        view.data = data + start*numChannels;
        view.numChannels = numChannels;
        view.numFrames = numFrames;
        return view;
    }
};

struct MotionClip {
    std::vector<float> data;
    uint32_t numChannels = 0;
    uint32_t numFrames = 0;

    MotionClip() = default;
    MotionClip(MotionClipView view) : numChannels(view.numChannels), numFrames(view.numFrames) {
        this->data = std::vector<float>(view.data, view.data + view.numFrames*view.numChannels);
    }

    static MotionClip fromSinglePose(glmx::pose_view pose_view, uint32_t numFrames) {
        MotionClip clip;
        clip.numFrames = numFrames;
        clip.numChannels = 3 + 4*pose_view.size;
        clip.data.resize(clip.numFrames*clip.numChannels);
        for (int f = 0; f < numFrames; f++) {
            clip.getFrame(f).v() = pose_view.v();
            for (int i = 0; i < pose_view.size; i++) {
                clip.getFrame(f).q(i) = pose_view.q(i);
            }
        }
        return clip;
    }

    static MotionClip empty(uint32_t numChannels, uint32_t numFrames) {
        MotionClip clip;
        clip.numChannels = numChannels;
        clip.numFrames = numFrames;
        clip.data.resize(clip.numFrames*clip.numChannels);
        return clip;
    }

    glmx::pose_view getFrame(uint32_t frameIdx) {
        assert(frameIdx >= 0);
        assert(frameIdx < numFrames);
        float* v_ptr = data.data() + frameIdx * numChannels;
        float* q_ptr = v_ptr + 3;
        return glmx::pose_view(v_ptr, q_ptr, (numChannels-3)/4);
    }

    void setFrame(uint32_t frameIdx, glmx::pose_view pose) {
        assert(frameIdx >= 0);
        assert(frameIdx < numFrames);
        float* v_ptr = data.data() + frameIdx * numChannels;
        float* q_ptr = v_ptr + 3;
        std::memcpy(v_ptr, pose.v_ptr, 3*sizeof(float));
        std::memcpy(q_ptr, pose.q_ptr, 4*pose.size*sizeof(float));
    }

    glm::vec3& rootPos(uint32_t frameIdx) {
        assert(frameIdx >= 0);
        assert(frameIdx < numFrames);
        float* v_ptr = data.data() + frameIdx * numChannels;
        return *((glm::vec3*)v_ptr);
    }

    const glm::vec3& rootPos(uint32_t frameIdx) const {
        assert(frameIdx >= 0);
        assert(frameIdx < numFrames);
        const float* v_ptr = data.data() + frameIdx * numChannels;
        return *((const glm::vec3*)v_ptr);
    }

    glm::quat& jointRot(uint32_t frameIdx, uint32_t jointIdx) {
        assert(frameIdx > 0);
        assert(frameIdx < numFrames);
        assert(jointIdx >= 0);
        assert(jointIdx < (numChannels-3)/4);
        float* q_ptr = data.data() + frameIdx * numChannels + 3 + 4*jointIdx;
        return *((glm::quat*)q_ptr);
    }

    const glm::quat& jointRot(uint32_t frameIdx, uint32_t jointIdx) const {
        assert(frameIdx > 0);
        assert(frameIdx < numFrames);
        assert(jointIdx >= 0);
        assert(jointIdx < (numChannels-3)/4);
        const float* q_ptr = data.data() + frameIdx * numChannels + 3 + 4*jointIdx;
        return *((const glm::quat*)q_ptr);
    };

    MotionClipView getView() {
        MotionClipView view;
        view.data = data.data();
        view.numChannels = numChannels;
        view.numFrames = numFrames;
        return view;
    }

    MotionClipView slice(uint32_t start, uint32_t end) {
        MotionClipView view;
        assert(start >= 0);
        assert(end <= numFrames);
        assert(start < end);
        view.data = data.data() + start*numChannels;
        view.numChannels = numChannels;
        view.numFrames = end - start;
        return view;
    }
};

#endif //DEEPMIMIC_MOTIONCLIP_H

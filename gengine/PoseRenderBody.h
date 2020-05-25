//
// Created by lasagnaphil on 19. 9. 23..
//

#ifndef GENGINE_POSERENDERBODY_H
#define GENGINE_POSERENDERBODY_H

#include "glmx/pose.h"
#include "glmx/common.h"
#include "Mesh.h"
#include "anim/BVHData.h"
#include "PhongRenderer.h"
#include "PBRenderer.h"
#include "DebugRenderer.h"

#include <glm/gtx/transform.hpp>

struct PoseRenderBody {
    std::vector<Ref<Mesh>> meshes;
    std::vector<Ref<PhongMaterial>> materials;

    static PoseRenderBody createAsBoxes(const PoseTree& poseTree, float width, Ref<PhongMaterial> material);
};

void renderMotionClip(PhongRenderer& renderer, DebugRenderer& imRenderer,
                             glmx::pose_view poseState, const PoseTree& poseTree, const PoseRenderBody& body,
                             const glm::mat4& globalTrans = glm::mat4(1.0f), bool debug = false);

struct PoseRenderBodyPBR {
    std::vector<Ref<Mesh>> meshes;
    std::vector<Ref<PBRMaterial>> materials;
    std::vector<glm::vec3> offsets;
    std::vector<glm::vec3> directions;

    static PoseRenderBodyPBR createAsBoxes(const PoseTree& poseTree, float width, Ref<PBRMaterial> material);

};

void renderMotionClip(PBRenderer& renderer, DebugRenderer& imRenderer,
                             glmx::pose_view poseState, const PoseTree& poseTree, const PoseRenderBodyPBR& body,
                             const glm::mat4& globalTrans = glm::mat4(1.0f), bool debug = false);

void renderMotionClipComplex(PBRenderer& renderer, DebugRenderer& imRenderer,
        glmx::pose_view poseState, const PoseTree& poseTree,
        const PoseRenderBodyPBR& body,
        const glm::mat4& globalTrans = glm::mat4(1.0f),
        bool debug = false);

#endif //GENGINE_POSERENDERBODY_H

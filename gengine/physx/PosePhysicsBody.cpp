//
// Created by lasagnaphil on 19. 12. 17..
//

#include "PosePhysicsBody.h"

void PosePhysicsBody::init(PhysicsWorld& world, const PoseTree& poseTree, const PosePhysicsBodySkel& skel) {
    // Initialize Articulation
    articulation = world.physics->createArticulationReducedCoordinate();
    articulation->setSolverIterationCounts(32);

    std::stack<uint32_t> recursionStack;
    recursionStack.push(0);
    while (!recursionStack.empty()) {
        uint32_t jointIdx = recursionStack.top();
        recursionStack.pop();

        const PosePhysicsBodySkel::Joint& joint = skel.getJoint(jointIdx);
        const PosePhysicsBodySkel::Joint& parent = skel.getJoint(joint.parentIdx);
        const PoseTreeNode& bvhNode = poseTree[joint.bvhIdx];

        PxArticulationLink* parentLink = nullptr;
        PxArticulationLink* link;
        if (jointIdx == 0) {
            link = articulation->createLink(nullptr, PxTransform(GLMToPx(joint.jointTrans)));
        }
        else {
            parentLink = nodeToLink[parent.bvhIdx];
            link = articulation->createLink(parentLink,
                                            PxTransform(GLMToPx(joint.jointTrans - parent.jointTrans)));

        }
        link->setName(joint.name.c_str());

        auto geometry = PxBoxGeometry(GLMToPx(joint.size/2.f));
        PxShape* shape = PxRigidActorExt::createExclusiveShape(*link, geometry, *world.defaultMaterial);
        PxRigidBodyExt::updateMassAndInertia(*link, 1.0f);
        nodeToLink[joint.bvhIdx] = link;

        auto pxJoint = static_cast<PxArticulationJointReducedCoordinate*>(link->getInboundJoint());
        if (pxJoint) {
            glmx::transform origin;
            PxTransform pPose = PxTransform(GLMToPx(joint.jointTrans - parent.bodyTrans));
            PxTransform cPose = PxTransform(GLMToPx(joint.jointTrans - joint.bodyTrans));
            pxJoint->setParentPose(pPose);
            pxJoint->setChildPose(cPose);

            pxJoint->setJointType(PxArticulationJointType::eSPHERICAL);
            pxJoint->setMotion(PxArticulationAxis::eTWIST, PxArticulationMotion::eFREE);
            pxJoint->setMotion(PxArticulationAxis::eSWING1, PxArticulationMotion::eFREE);
            pxJoint->setMotion(PxArticulationAxis::eSWING2, PxArticulationMotion::eFREE);
            /*
            pxJoint->setLimit(PxArticulationAxis::eTWIST, -M_PI/2, M_PI/2);
            pxJoint->setLimit(PxArticulationAxis::eSWING1, -0.95f * M_PI, 0.95f * M_PI);
            pxJoint->setLimit(PxArticulationAxis::eSWING2, -0.95f * M_PI, 0.95f * M_PI);
            */
        }

        if (!joint.childIdx.empty()) {
            for (auto childIdx : joint.childIdx) {
                recursionStack.push(childIdx);
            }
        }
    }

    aggregate = world.physics->createAggregate(articulation->getNbLinks(), false);
    aggregate->addArticulation(*articulation);
    world.scene->addAggregate(*aggregate);

    std::vector<PxArticulationLink*> links;
    links.resize(articulation->getNbLinks());
    articulation->getLinks(links.data(), links.size());

    cache = articulation->createCache();

    dofStarts.resize(links.size());
    dofStarts[0] = 0; // We know that the root link does not have a joint
    for (uint32_t i = 1; i < links.size(); i++) {
        uint32_t llIndex = links[i]->getLinkIndex();
        uint32_t dofs = links[i]->getInboundJointDof();
        dofStarts[llIndex] = dofs;
    }
    dofCount = 0;
    for(PxU32 i = 1; i < links.size(); ++i)
    {
        PxU32 dofs = dofStarts[i];
        dofStarts[i] = dofCount;
        dofCount += dofs;
    }
}

void PosePhysicsBody::setPose(const glmx::pose& pose, const PoseTree& poseTree) {
    PxArticulationCacheFlags flags = PxArticulationCache::eALL;
    articulation->copyInternalStateToCache(*cache, flags);

    cache->rootLinkData->transform = PxTransform(GLMToPx(pose.v()), GLMToPx(pose.q(0)));

    for (uint32_t i = 1; i < pose.size(); i++) {
        glm::vec3 v = glmx::log(pose.q(i));
        uint32_t li = nodeToLink[i]->getLinkIndex();
        cache->jointPosition[dofStarts[li]] = v.x;
        cache->jointPosition[dofStarts[li] + 1] = v.y;
        cache->jointPosition[dofStarts[li] + 2] = v.z;
    }

    articulation->applyCache(*cache, flags);
}

void PosePhysicsBody::getPose(glmx::pose& pose, const PoseTree& poseTree) {
    PxArticulationCacheFlags flags = PxArticulationCache::eROOT | PxArticulationCache::ePOSITION;
    articulation->copyInternalStateToCache(*cache, flags);

    pose.v() = PxToGLM(nodeToLink[0]->getGlobalPose().p);
    pose.q(0) = PxToGLM(nodeToLink[0]->getGlobalPose().q);

    for (uint32_t i = 1; i < pose.size(); i++) {
        glm::vec3 v;
        uint32_t li = nodeToLink[i]->getLinkIndex();
        v.x = cache->jointPosition[dofStarts[li]];
        v.y = cache->jointPosition[dofStarts[li]+1];
        v.z = cache->jointPosition[dofStarts[li]+2];
        pose.q(i) = glmx::exp(v);
    }
}

void PosePhysicsBody::setPoseVelocityFromTwoPoses(const glmx::pose& p1, const glmx::pose& p2, float dt) {
    PxArticulationCacheFlags flags = PxArticulationCache::eVELOCITY;
    for (uint32_t i = 1; i < p1.size(); i++) {
        glm::quat q = glm::inverse(p1.q(i)) * p2.q(i);

        glm::vec3 v = glmx::quatToEuler(q, EulOrdXYZr);
        uint32_t li = nodeToLink[i]->getLinkIndex();
        cache->jointVelocity[dofStarts[li]] = v.x / dt;
        cache->jointVelocity[dofStarts[li] + 1] = v.y / dt;
        cache->jointVelocity[dofStarts[li] + 2] = v.z / dt;
    }
    articulation->applyCache(*cache, flags);
}

void PosePhysicsBody::setRoot(const glmx::transform& rootTrans) {
    PxArticulationCacheFlags flags = PxArticulationCache::eROOT;
    articulation->copyInternalStateToCache(*cache, flags);

    cache->rootLinkData->transform.p = GLMToPx(rootTrans.v);
    cache->rootLinkData->transform.q = GLMToPx(rootTrans.q);

    articulation->applyCache(*cache, flags);
}

void PosePhysicsBody::renderImGui() {
    PxArticulationCacheFlags flags = PxArticulationCache::eROOT | PxArticulationCache::ePOSITION;
    articulation->copyInternalStateToCache(*cache, flags);

    bool edited = false;
    edited |= ImGui::DragFloat3("Root Position", (float*)&cache->rootLinkData->transform.p, 0.01f);
    glm::quat q = PxToGLM(cache->rootLinkData->transform.q);
    glm::vec3 euler = glmx::quatToEuler(q, EulOrdXYZr);
    ImGui::DragFloat3("Root Rotation", (float*)&euler, 0.01f);
    cache->rootLinkData->transform.q = GLMToPx(glmx::eulerToQuat(euler, EulOrdXYZr));

    for (auto [bvhIdx, link] : nodeToLink) {
        uint32_t li = link->getLinkIndex();
        glm::vec3 v;
        v.x = cache->jointPosition[dofStarts[li]];
        v.y = cache->jointPosition[dofStarts[li] + 1];
        v.z = cache->jointPosition[dofStarts[li] + 2];
        bool linkEdited = ImGui::DragFloat3(link->getName(), (float*)&v, 0.01f);
        if (linkEdited) {
            cache->jointPosition[dofStarts[li]] = v.x;
            cache->jointPosition[dofStarts[li] + 1] = v.y;
            cache->jointPosition[dofStarts[li] + 2] = v.z;
        }
        edited |= linkEdited;
    }

    if (edited) {
        articulation->applyCache(*cache, PxArticulationCache::eROOT | PxArticulationCache::ePOSITION);
    }
}

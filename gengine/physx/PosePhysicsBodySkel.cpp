//
// Created by lasagnaphil on 19. 12. 17..
//

#include "PosePhysicsBodySkel.h"

PosePhysicsBodySkel PosePhysicsBodySkel::fromFile(const std::string& filename, const PoseTree& poseTree) {
    using namespace tinyxml2;

    PosePhysicsBodySkel map;

    XMLDocument doc;
    doc.LoadFile(filename.c_str());

    XMLNode* root = doc.FirstChild();
    if (root == nullptr) {
        fprintf(stderr, "Error while reading XML file %s!", filename.c_str());
        exit(EXIT_FAILURE);
    }
    XMLElement* jointNode = root->FirstChildElement("Joint");
    if (jointNode == nullptr) {
        fprintf(stderr, "No Joint element found in XML file %s!", filename.c_str());
        exit(EXIT_FAILURE);
    }

    auto findAttributeVec3 = [](XMLElement* node, const char* name) {
        const XMLAttribute* attr = node->FindAttribute(name);
        if (attr) {
            return stringToVec3(attr->Value());
        }
        else {
            return glm::vec3();
        }
    };

    map.skeletonName = jointNode->FindAttribute("name")->Value();

    uint32_t jointIdx = 0;
    while (jointNode != nullptr) {
        Joint joint;
        joint.name = jointNode->FindAttribute("name")->Value();
        std::string parentName = jointNode->FindAttribute("parent_name")->Value();
        if (parentName == "None") {
            joint.parentIdx = 0;
        }
        else {
            if (map.jointNameMapping.count(parentName) > 0) {
                joint.parentIdx = map.jointNameMapping[parentName];
            }
            else {
                std::cerr << "Invalid parent joint name: " << parentName << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        joint.size = stringToVec3(jointNode->FindAttribute("size")->Value());
        joint.mass = jointNode->FindAttribute("mass")->FloatValue();
        std::string bvhNodeName = jointNode->FindAttribute("bvh")->Value();
        joint.bvhIdx = poseTree.findIdx(bvhNodeName);
        if (joint.bvhIdx == (uint32_t) -1) {
            std::cerr << "Invalid BVH node name: " << bvhNodeName << std::endl;
            exit(EXIT_FAILURE);
        }
        XMLElement* bodyPosNode = jointNode->FirstChildElement("BodyPosition");
        joint.xdir = stringToVec3(bodyPosNode->FindAttribute("xdir")->Value());
        joint.bodyTrans = stringToVec3(bodyPosNode->FindAttribute("translation")->Value());
        XMLElement* jointPosNode = jointNode->FirstChildElement("JointPosition");
        joint.jointTrans = stringToVec3(jointPosNode->FindAttribute("translation")->Value());

        bool shapeExists = false;
        XMLElement* capsuleNode = jointNode->FirstChildElement("Capsule");
        if (capsuleNode) {
            shapeExists = true;
            joint.shape.type = ShapeType::Capsule;
            joint.shape.capsule.direction = stringToVec3(capsuleNode->FindAttribute("direction")->Value());
            joint.shape.capsule.offset = findAttributeVec3(capsuleNode, "offset");
            joint.shape.capsule.radius = capsuleNode->FindAttribute("radius")->FloatValue();
            joint.shape.capsule.height = capsuleNode->FindAttribute("height")->FloatValue();
        }
        XMLElement* cylinderNode = jointNode->FirstChildElement("Cylinder");
        if (cylinderNode) {
            shapeExists = true;
            joint.shape.type = ShapeType::Cylinder;
            joint.shape.cylinder.direction = stringToVec3(cylinderNode->FindAttribute("direction")->Value());
            joint.shape.cylinder.offset = findAttributeVec3(cylinderNode, "offset");
            joint.shape.cylinder.radius = cylinderNode->FindAttribute("radius")->FloatValue();
            joint.shape.cylinder.height = cylinderNode->FindAttribute("height")->FloatValue();
        }
        XMLElement* sphereNode = jointNode->FirstChildElement("Sphere");
        if (sphereNode) {
            shapeExists = true;
            joint.shape.type = ShapeType::Box;
            joint.shape.sphere.offset = findAttributeVec3(sphereNode, "offset");
            joint.shape.sphere.radius = sphereNode->FindAttribute("radius")->FloatValue();
        }
        XMLElement* boxNode = jointNode->FirstChildElement("Box");
        if (boxNode) {
            shapeExists = true;
            joint.shape.type = ShapeType::Box;
            joint.shape.box.offset = findAttributeVec3(boxNode, "offset");
            joint.shape.box.size = stringToVec3(boxNode->FindAttribute("size")->Value());
        }

        if (!shapeExists) {
            joint.shape.type = ShapeType::None;
        }

        map.jointNameMapping[joint.name] = jointIdx;
        map.bvhMapping[joint.bvhIdx] = jointIdx;

        map.joints.push_back(joint);
        jointIdx++;
        jointNode = jointNode->NextSiblingElement("Joint");
    }

    for (int i = 0; i < map.joints.size(); i++) {
        Joint& joint = map.joints[i];
        if (joint.parentIdx == i) { continue; }
        Joint& parentJoint = map.joints[joint.parentIdx];
        parentJoint.childIdx.push_back(i);
    }

    return map;
}

PoseRenderBodyPBR
createFromSkel(const PoseTree& poseTree, PosePhysicsBodySkel& skel, const std::vector<Ref<PBRMaterial>>& materials) {

    assert(poseTree.numNodes == materials.size());
    PoseRenderBodyPBR body;
    body.materials = materials;

    body.meshes.resize(poseTree.numNodes);
    body.offsets.resize(poseTree.numNodes);
    body.directions.resize(poseTree.numNodes);

    for (int bvhIdx = 0; bvhIdx < poseTree.numNodes; bvhIdx++) {
        if (skel.bvhMapping.count(bvhIdx)) {
            uint32_t i = skel.bvhMapping[bvhIdx];
            auto& shape = skel.joints[i].shape;
            switch(shape.type) {
                case PosePhysicsBodySkel::ShapeType::None: {
                    body.meshes[i] = {};
                    body.offsets[i] = {};
                    body.directions[i] = {};
                    break;
                }
                case PosePhysicsBodySkel::ShapeType::Box: {
                    body.meshes[i] = Mesh::makeCube(shape.box.size);
                    body.offsets[i] = shape.box.offset;
                    body.directions[i] = {};
                    break;
                }
                case PosePhysicsBodySkel::ShapeType::Capsule: {
                    body.meshes[i] = Mesh::makeCapsule(shape.capsule.radius, shape.capsule.height);
                    body.offsets[i] = shape.capsule.offset;
                    body.directions[i] = shape.capsule.direction;
                    break;
                }
                case PosePhysicsBodySkel::ShapeType::Cylinder: {
                    body.meshes[i] = Mesh::makeCylinder(18, shape.cylinder.radius, shape.cylinder.height);
                    body.offsets[i] = shape.cylinder.offset;
                    body.directions[i] = shape.cylinder.direction;
                    break;
                }
                case PosePhysicsBodySkel::ShapeType::Sphere: {
                    body.meshes[i] = Mesh::makeSphere(shape.sphere.radius);
                    body.offsets[i] = shape.sphere.offset;
                    body.directions[i] = {};
                    break;
                }
            }
        }
        else {
            body.meshes[bvhIdx] = {};
        }
    }
    return body;
}

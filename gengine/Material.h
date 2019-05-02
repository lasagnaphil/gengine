//
// Created by lasagnaphil on 19. 3. 8.
//

#ifndef MOTION_EDITING_MATERIAL_H
#define MOTION_EDITING_MATERIAL_H

#include <glm/vec4.hpp>
#include <glad/glad.h>
#include "Texture.h"

struct Material {
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float shininess = 32.0f;
    Ref<Texture> texDiffuse = {};
    Ref<Texture> texSpecular = {};
};

#endif //MOTION_EDITING_MATERIAL_H

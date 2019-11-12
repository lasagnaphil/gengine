//
// Created by lasagnaphil on 19. 11. 12..
//

#ifndef GENGINE_RECT_H
#define GENGINE_RECT_H

#include <glm/vec2.hpp>

namespace glmx {
    struct rect {
        glm::vec2 min; // upper left
        glm::vec2 max; // lower right
    };

    struct box {
        glm::vec3 min;
        glm::vec3 max;
    };
}

#endif //GENGINE_RECT_H

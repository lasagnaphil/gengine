//
// Created by lasagnaphil on 19. 3. 14.
//

#ifndef MOTION_EDITING_TEXTURE_H
#define MOTION_EDITING_TEXTURE_H

#include <glad/glad.h>
#include "Image.h"
#include "Storage.h"
#include <string>
#include "IDisposable.h"

struct Texture : IDisposable {
    GLuint id = 0;
    GLuint width, height;
    GLuint internalFormat;
    GLuint imageFormat;

    GLuint wrapS;
    GLuint wrapT;
    GLuint filterMin;
    GLuint filterMax;

    std::string imagePath;

    Texture() = default;
    explicit Texture(Ref<Image> image);
    void dispose();

    void bind();
};

#endif //MOTION_EDITING_TEXTURE_H

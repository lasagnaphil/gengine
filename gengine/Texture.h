//
// Created by lasagnaphil on 19. 3. 14.
//

#ifndef MOTION_EDITING_TEXTURE_H
#define MOTION_EDITING_TEXTURE_H

#include <glad/glad.h>
#include "Image.h"
#include "GenAllocator.h"
#include <string>
#include "IDisposable.h"

struct Texture {
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
    static Ref<Texture> fromImage(Ref<Image> image);
    static Ref<Texture> fromNew(uint32_t width, uint32_t height);
    void loadFromImage(Ref<Image> image);
    void dispose();

    void bind();
};

#endif //MOTION_EDITING_TEXTURE_H

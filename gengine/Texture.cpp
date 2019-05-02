//
// Created by lasagnaphil on 19. 3. 14.
//

#include <iostream>
#include "Texture.h"

Texture::Texture(Ref<Image> image) {
    imagePath = image->path;

    width = image->width;
    height = image->height;
    wrapS = GL_REPEAT;
    wrapT = GL_REPEAT;
    filterMin = GL_LINEAR_MIPMAP_LINEAR;
    filterMax = GL_LINEAR;

    int nrComponents = image->nrChannels;
    if (nrComponents == 1) {
        imageFormat = GL_RED;
        internalFormat = GL_RED;
    }
    else if (nrComponents == 3) {
        imageFormat = GL_RGB;
        internalFormat = GL_RGB;
    }
    else if (nrComponents == 4) {
        imageFormat = GL_RGBA;
        internalFormat = GL_RGBA;
    }

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexImage2D(GL_TEXTURE_2D,
                 0, internalFormat,
                 width, height,
                 0, imageFormat, GL_UNSIGNED_BYTE, image->data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMin);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMax);

    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}


void Texture::bind() {
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::dispose() {
    glDeleteTextures(1, &id);
}

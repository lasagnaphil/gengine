//
// Created by lasagnaphil on 19. 3. 14.
//

#include <stb_image.h>

#include <iostream>

#include "Image.h"

Image::Image(const char *filename, int desiredChannels) : desiredChannels(desiredChannels) {
    path = std::string(filename);

    data = stbi_load(path.c_str(), &width, &height, &nrChannels, desiredChannels);
    if (!data) {
        std::cerr << "Failed to load image " << path.c_str() << "!\n";
        exit(EXIT_FAILURE);
    }
}

void Image::dispose() {
    if (data) {
        stbi_image_free(data);
    }
}
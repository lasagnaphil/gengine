//
// Created by lasagnaphil on 20. 1. 2..
//

#include "implot.h"

#include <glad/glad.h>
#include <fmt/core.h>

using namespace plt;

ImPlot2DContext ImPlot2DContext::create(float sizeX, float sizeY) {
    ImPlot2DContext ctx;
    ctx.sizeX = sizeX;
    ctx.sizeY = sizeY;

    glGenFramebuffers(1, &ctx.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, ctx.fbo);

    glGenTextures(1, &ctx.tex);
    glBindTexture(GL_TEXTURE_2D, ctx.tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sizeX, sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ctx.tex, 0);

    glGenRenderbuffers(1, &ctx.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, ctx.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, sizeX, sizeY);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ctx.rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fmt::print(stderr, "Error: Framebuffer is not complete!\n");
        return ctx;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return ctx;
}

void ImPlot2DContext::show() {
    // TODO: We should protect this code using a mutex
    //       so that nobody would try to use the image before rendering is complete
    renderFinished = false;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // TODO: actually render all the stuff

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // clear the buffers after rendering is finished
    clear();

    renderFinished = true;
}

void ImPlot2DContext::saveToImage(const std::string& filename) {
    // TODO
}

ImPlot2DMouseResult ImPlot2DContext::mouseClick(int x, int y, uint8_t mouseButton) {
    // TODO
    return ImPlot2DMouseResult();
}

ImPlot2DMouseResult ImPlot2DContext::mouseHover(int x, int y) {
    // TODO
    return ImPlot2DMouseResult();
}

void ImPlot2DContext::mouseDrag(int relX, int relY) {
    // TODO
}


//
// Created by lasagnaphil on 20. 1. 2..
//

#include "implot.h"

#include <glad/glad.h>
#include <fmt/core.h>
#include <Shader.h>
#include <glm/gtc/matrix_transform.hpp>
#include "shaders/plot_point2d.vert.h"
#include "shaders/plot_point2d.frag.h"

using namespace plt;

ImPlot2DContext ImPlot2DContext::create(float sizeX, float sizeY) {
    ImPlot2DContext ctx;
    ctx.sizeX = sizeX;
    ctx.sizeY = sizeY;

    // Create framebuffer and framebuffer texture
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

    // Create vaos and vbos for point / line

    glGenVertexArrays(1, &ctx.pointVAO);
    glBindVertexArray(ctx.pointVAO);

    glGenBuffers(1, &ctx.pointVBO);
    glBindBuffer(GL_ARRAY_BUFFER, ctx.pointVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point2D) * 1024, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point2D), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point2D), (void*)offsetof(Point2D, color));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Point2D), (void*)offsetof(Point2D, size));

    glBindVertexArray(0);

    // Compile shaders

    ctx.point2d_shader = Resources::make<Shader>();
    ctx.point2d_shader->compileFromString(plot_point2d_vert_shader, plot_point2d_frag_shader);

    ctx.projMat = glm::ortho(0.0f, sizeX, 0.0f, sizeY, -1.0f, 1.0f);
    ctx.viewMat = glm::mat4(1.0f);

    return ctx;
}

void ImPlot2DContext::show() {
    // TODO: We should protect this code using a mutex
    //       so that nobody would try to use the image before rendering is complete
    renderFinished = false;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    point2d_shader->use();
    point2d_shader->setMat4("view", viewMat);
    point2d_shader->setMat4("proj", projMat);

    glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Point2D) * points2D.size(), points2D.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(pointVAO);
    glDrawArrays(GL_POINTS, 0, points2D.size());
    glBindVertexArray(0);

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


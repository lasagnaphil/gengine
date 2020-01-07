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
#include <imgui.h>
#include <SDL2/SDL_mouse.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point2D) * 32768, nullptr, GL_DYNAMIC_DRAW);
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

    ctx.projMat = glm::ortho(0.0f, sizeX, sizeY, 0.0f, -1.0f, 1.0f);
    ctx.viewMat = glm::mat4(1.0f);

    return ctx;
}

ImPlot2DResult ImPlot2DContext::show() {
    ImPlot2DResult res;

    // TODO: We should protect this code using a mutex
    //       so that nobody would try to use the image before rendering is complete
    renderFinished = false;

    if (autoscaleEnabled) { autoscale(); }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, sizeX, sizeY);

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
    auto displaySize = ImGui::GetIO().DisplaySize;
    glViewport(0, 0, displaySize.x, displaySize.y);

    // Now for the imgui part
    ImGui::Image((void*)tex, ImVec2(sizeX, sizeY));

    ImVec2 mouse = ImGui::GetIO().MousePos;
    auto frameMin = ImGui::GetItemRectMin();
    auto frameMax = ImGui::GetItemRectMax();
    float screenToFrameX = 1.0f / viewMat[0][0];
    float screenToFrameY = 1.0f / viewMat[1][1];
    glm::vec4 relMousePos = viewMat * glm::vec4(mouse.x - frameMin.x, mouse.y - frameMin.y, 0.0f, 1.0f);
    float mX = bounds.min.x + screenToFrameX * (mouse.x - frameMin.x);
    float mY = bounds.max.y - screenToFrameY * (mouse.y - frameMin.y);

    int selected = -1;
    float distMin2 = std::numeric_limits<float>::max();
    for (int i = 0; i < points2D.size(); i++) {
        float x = points2D[i].pos.x;
        float y = points2D[i].pos.y;
        float dist2 = (x - mX) * (x - mX) + (y - mY) * (y - mY);
        if (dist2 < distMin2) {
            distMin2 = dist2;
            selected = i;
        }
    }
    float threshold = grabRadius * grabRadius * screenToFrameX * screenToFrameY;

    if (selected != -1 && distMin2 < threshold) {
        ImGui::SetTooltip("(%4.3f, %4.3f)", points2D[selected].pos.x, points2D[selected].pos.y);
        if (ImGui::IsItemClicked()) {
            res.clickedPointIdx = selected;
        }
    }

    if (ImGui::IsMouseDragging(1)) {
        ImVec2 dm = ImGui::GetIO().MouseDelta;
        float scaleX = 1.0f / viewMat[0][0];
        float scaleY = 1.0f / viewMat[1][1];
        bounds.min -= glm::vec2(scaleX * dm.x, -scaleY * dm.y);
        bounds.max -= glm::vec2(scaleX * dm.x, -scaleY * dm.y);
        autoscaleEnabled = false;
    }

    if (ImGui::Button("Auto-scale")) {
        autoscale();
    }
    ImGui::SameLine();
    if (ImGui::Button("Zoom in")) {
        autoscaleEnabled = false;
        bounds.min = (bounds.min + (bounds.min + bounds.max)/2.0f) / 2.0f;
        bounds.max = (bounds.max + (bounds.min + bounds.max)/2.0f) / 2.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Zoom out")) {
        autoscaleEnabled = false;
        bounds.min = 2.0f*bounds.min - (bounds.min + bounds.max)/2.0f;
        bounds.max = 2.0f*bounds.max - (bounds.min + bounds.max)/2.0f;
    }

    glm::vec2 scale = {sizeX / (bounds.max.x - bounds.min.x), sizeY / (bounds.max.y - bounds.min.y)};
    if (scale.x == 0.0f) {
        scale.x = 1.0f;
    }
    if (scale.y == 0.0f) {
        scale.y = 1.0f;
    }
    viewMat = glm::mat4(1.0f);
    viewMat = glm::scale(viewMat, {scale.x, scale.y, 1.0f});
    viewMat = glm::translate(viewMat, {-bounds.min.x, -bounds.min.y, 0});

    // clear the buffers after rendering is finished
    clear();

    renderFinished = true;

    res.mousePos = {mX, mY};
    return res;
}

void ImPlot2DContext::autoscale() {
    // Calculate the view mat to show all points
    bounds.min.x = std::numeric_limits<float>::max();
    bounds.max.x = std::numeric_limits<float>::min();
    bounds.min.y = std::numeric_limits<float>::max();
    bounds.max.y = std::numeric_limits<float>::min();
    for (auto& point : points2D) {
        if (point.pos.x < bounds.min.x) bounds.min.x = point.pos.x;
        if (point.pos.x > bounds.max.x) bounds.max.x = point.pos.x;
        if (point.pos.y < bounds.min.y) bounds.min.y = point.pos.y;
        if (point.pos.y > bounds.max.y) bounds.max.y = point.pos.y;
    }
    float borderX = 0.1f * (bounds.max.x - bounds.min.x);
    float borderY = 0.1f * (bounds.max.y - bounds.min.y);
    bounds.min.x -= borderX; bounds.max.x += borderX; bounds.min.y -= borderY; bounds.max.y += borderY;


    autoscaleEnabled = true;
}

void ImPlot2DContext::saveToImage(const std::string& filename) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLubyte* pixels = new GLubyte[int(sizeX) * int(sizeY) * 3];
    glReadPixels(0, 0, sizeX, sizeY, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    stbi_write_png(filename.c_str(), sizeX, sizeY, 3, pixels, 3*sizeX);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    delete[] pixels;
}


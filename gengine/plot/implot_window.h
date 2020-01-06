//
// Created by lasagnaphil on 20. 1. 2..
//

#ifndef GENGINE_IMPLOT_WINDOW_H
#define GENGINE_IMPLOT_WINDOW_H

#include <imgui.h>

#include "plot/implot.h"
#include "App.h"
#include "InputManager.h"

struct ImPlotApp : public App {
    plt::ImPlot2DContext ctx;

    ImPlotApp() : App(false) {}

    void loadResources() override {
        // TODO: need static camera
        FlyCamera* camera = initCamera<FlyCamera>();
    }

    void update(float dt) override {
    }

    void render() override {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("ImPlotApp");
        if (ctx.renderFinished) {
            ImGui::Image((void*)ctx.tex, ImVec2(ctx.sizeX, ctx.sizeY));

            auto mousePos = ImGui::GetMousePos();
            if (ImGui::IsItemClicked()) {
                ctx.mouseClick(mousePos.x, mousePos.y, SDL_BUTTON_LEFT);
            }
            if (ImGui::IsItemHovered()) {
                if (ImGui::IsMouseDragging()) {
                    auto dd = ImGui::GetMouseDragDelta();
                    ctx.mouseDrag(dd.x, dd.y);
                }
                else {
                    ctx.mouseHover(mousePos.x, mousePos.y);
                }
            }
        }

        ImGui::End();
    }

    plt::ImPlot2DContext& plt() { return ctx; }
};

#endif //GENGINE_IMPLOT_WINDOW_H
